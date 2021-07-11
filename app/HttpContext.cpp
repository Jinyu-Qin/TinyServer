#include "HttpContext.h"
#include "TimeStamp.h"
#include "Buffer.h"
#include "TcpConnection.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <cassert>
#include <cstring>

HttpContext::HttpContext(TcpConnectionPtr conn)
    : conn_(conn)
    , requestDecodeState_(kDecodeRequestDone)
    , requestBodyEnd_(nullptr)
    , requestBodyRemainingSize_(0)
    , responseBuffer_(new Buffer) {
}

HttpContext::~HttpContext() {
}

HttpContext::TcpConnectionPtr HttpContext::getTcpConnection() const {
    return conn_;
}

void HttpContext::process(BufferPtr message, TimeStamp received) {
    bool nullRequest = !static_cast<bool>(request_);
    bool requestHandled = requestDecodeState_ == kDecodeRequestDone || requestDecodeState_ == kDecodeRequestError;
    assert((nullRequest && requestHandled) || (!nullRequest && !requestHandled));

    if(!request_) {
        // request为nullptr，重启状态机
        request_.reset(new HttpRequest);
        requestDecodeState_ = kDecodeRequestLine;
    }

    // 解码request
    decodeHttpRequest(request_, message);
    if(requestDecodeState_ == kDecodeRequestError) {
        // 解析request出错，清空缓冲区
        message->hasRead(message->readableSize());
        handleRequestError();
        return ;
    } else if(!requestDecodeState_ == kDecodeRequestDone) {
        // 解析未全部完成
        return ;
    }


    // 处理请求完毕后是否关闭连接
    // FIXME ignore case?
    const auto & connection = request_->getHeader("Connection");
    bool closeAfterService = !connection.empty() && connection == "close";

    // 处理请求
    if(serviceCallback_) {
        try {
            response_.reset(new HttpResponse);
            serviceCallback_(request_, response_);
        } catch(...) {
            // 处理请求过程中出错
            handleProcessError();
            return ;
        }
    } else {
        // 没有设置request的处理函数
        handleProcessError();
        return ;
    }

    // 编码response
    encodeHttpResponse(response_, responseBuffer_.get());
    // 发送response
    conn_->send(responseBuffer_.get());
    // 释放request
    request_.reset();

    // 关闭连接
    if(closeAfterService) {
        conn_->shutdown();
    }
}

void HttpContext::setServiceCallback(ServiceCallback callback) {
    serviceCallback_ = callback;
}

const char * HttpContext::findCRLF(BufferPtr message) {
    const char * begin = message->readBegin();
    ssize_t size = message->readableSize();
    const char * end = begin + size;

    for(const char * it = begin; it != end; ++it) {
        if(*it == '\r' && it + 1 < end && *(it + 1) == '\n') {
            return it;
        }
    }

    return nullptr;
}

const char * HttpContext::findSpace(BufferPtr message) {
    const char * begin = message->readBegin();
    ssize_t size = message->readableSize();
    const char * end = begin + size;

    for(const char * it = begin; it != end; ++it) {
        if(*it == ' ') {
            return it;
        }
    }

    return nullptr;
}

const char * HttpContext::findColon(BufferPtr message) {
    const char * begin = message->readBegin();
    ssize_t size = message->readableSize();
    const char * end = begin + size;

    for(const char * it = begin; it != end; ++it) {
        if(*it == ':' && it + 1 < end && *(it + 1) == ' ') {
            return it;
        }
    }

    return nullptr;
}

const char * HttpContext::findQuestionMark(BufferPtr message) {
    const char * begin = message->readBegin();
    ssize_t size = message->readableSize();
    const char * end = begin + size;

    for(const char * it = begin; it != end; ++it) {
        if(*it == '?') {
            return it;
        }
    }

    return nullptr;
}

void HttpContext::decodeHttpRequest(HttpRequestPtr request, BufferPtr message) {
    if(requestDecodeState_ == kDecodeRequestLine) {
        // 请求行
        const char * lineBegin = message->readBegin();
        const char * lineEnd = findCRLF(message);
        if(lineEnd == nullptr) {
            return;
        }

        // 方法
        const char * begin = lineBegin;
        const char * end = findSpace(message);
        if(end == nullptr) {
            // 没找到分隔的空格
            requestDecodeState_ = kDecodeRequestError;
            return;
        }
        request->setMethod(begin, end);
        message->hasRead(end - begin + 1);
        if(request->method() == HttpMethod::kInvalid) {
            // 方法错误或不支持
            requestDecodeState_ = kDecodeRequestError;
            return;
        }

        // URL
        begin = end + 1;
        end = findSpace(message);
        if(end == nullptr) {
            // 没找到分隔的空格
            requestDecodeState_ = kDecodeRequestError;
            return;
        }
        if(request->method() == HttpMethod::kPost) {
            // POST
            request->setPath(begin, end);
            message->hasRead(end - begin + 1);
        } else if(request->method() == HttpMethod::kGet) {
            // GET
            const char * it = findQuestionMark(message);
            if(it != nullptr) {
                request->setPath(begin, it);
                request->setQuery(it + 1, end);
            } else {
                request->setPath(begin, end);
            }
            message->hasRead(end - begin + 1);
        }

        // HTTP版本
        begin = end + 1;
        end = lineEnd;
        request->setVersion(begin, end);
        message->hasRead(end - begin + 2);
        if(request->version() == HttpVersion::kUnknown) {
            // 协议错误或不支持
            requestDecodeState_ = kDecodeRequestError;
            return;
        }

        requestDecodeState_ = kDecodeRequestHeaders;
    }

    if(requestDecodeState_ == kDecodeRequestHeaders) {
        // 请求首部
        while(true) {
            const char * lineBegin = message->readBegin();
            const char * lineEnd = findCRLF(message);
            if(lineEnd == nullptr) {
                return;
            }

            if(lineBegin == lineEnd) {
                // 空行
                if(!request->headers().empty()) {
                    message->hasRead(2);
                    requestDecodeState_ = kDecodeRequestBody;
                    break;
                } else {
                    // 缺少首部
                    requestDecodeState_ = kDecodeRequestError;
                    return;
                }
            } else {
                // 首部行
                const char *it = findColon(message);
                if(it == nullptr) {
                    // 首部行缺少冒号
                    requestDecodeState_ = kDecodeRequestError;
                    return;
                }
                request->setHeader(std::string(lineBegin, it), std::string(it + 2, lineEnd));
                message->hasRead(lineEnd - lineBegin + 2);
            }
        }
    }

    if(requestDecodeState_ == kDecodeRequestBody) {
        // 请求体
        if(request->method() == HttpMethod::kGet) {
            // GET没有请求体
            requestDecodeState_ = kDecodeRequestDone;
        } else if(request->method() == HttpMethod::kPost) {
            // POST
            const char * begin = message->readBegin();
            if(requestBodyEnd_ == nullptr) {
                // 读取Content-Length字段
                const auto & contentLength = request->getHeader("Content-Length");
                if(contentLength.empty()) {
                    // POST必须包含Content-Length字段
                    requestDecodeState_ = kDecodeRequestError;
                    return;
                }
                requestBodyRemainingSize_ = std::stol(contentLength);
                requestBodyEnd_ = begin + requestBodyRemainingSize_;
            }

            ssize_t len = message->readableSize();
            if(len > requestBodyRemainingSize_) {
                len = requestBodyRemainingSize_;
            }
            request->appendBody(begin, begin + len);
            message->hasRead(len);
            requestBodyRemainingSize_ -= len;

            if(requestBodyRemainingSize_ == 0) {
                requestBodyEnd_ = nullptr;
                requestDecodeState_ = kDecodeRequestDone;
            }
        }
    }
}

void HttpContext::encodeHttpResponse(HttpResponsePtr response, BufferPtr message) {
    // 版本号
    const auto & version = getVersionMessage(response->version());
    message->write(version.data(), version.size());
    message->write(space.data(), space.size());
    
    // 状态码
    std::string statusCode(std::to_string(static_cast<int>(response->statusCode())));
    message->write(statusCode.data(), statusCode.size());
    message->write(space.data(), space.size());

    // 状态信息
    const auto & statusMsg = response->statusMessage();
    message->write(statusMsg.data(), statusMsg.size());
    message->write(crlf.data(), crlf.size());

    // 首部
    const auto & headers = response->headers();
    for(const auto & header : headers) {
        const auto & key = header.first;
        const auto & value = header.second;

        message->write(key.data(), key.size());
        message->write(colon.data(), colon.size());
        message->write(value.data(), value.size());
        message->write(crlf.data(), crlf.size());
    }

    // 空行
    message->write(crlf.data(), crlf.size());


    // 响应体
    const auto & body = response->body();
    if(!body.empty()) {
        message->write(body.data(), body.size());
    }
}


void HttpContext::handleRequestError() {
    assert(requestDecodeState_ == kDecodeRequestError);

    response_ = generalResponse(HttpStatusCode::kBadRequest);
    encodeHttpResponse(response_, responseBuffer_.get());
    conn_->send(responseBuffer_.get());
    conn_->shutdown();

    request_.reset();
}

void HttpContext::handleProcessError() {
    response_ = generalResponse(HttpStatusCode::kInternalServerError);
    encodeHttpResponse(response_, responseBuffer_.get());
    conn_->send(responseBuffer_.get());
    conn_->shutdown();

    request_.reset();
}

const std::string & HttpContext::getStatusMessage(HttpStatusCode statusCode) {
    auto it = statusMessage_.find(statusCode);
    assert(it != statusMessage_.cend());
    return it->second;
}

const std::string & HttpContext::getVersionMessage(HttpVersion version) {
    auto it = versionMessage_.find(version);
    assert(it != versionMessage_.cend());
    return it->second;
}

const std::string & HttpContext::getMethodMessage(HttpMethod method) {
    auto it = methodMessage_.find(method);
    assert(it != methodMessage_.cend());
    return it->second;
}

HttpContext::HttpResponsePtr HttpContext::generalResponse(HttpStatusCode statusCode) {
    auto it = generalResponse_.find(statusCode);
    if(it != generalResponse_.cend()) {
        return it->second;
    }

    HttpResponsePtr response(std::make_shared<HttpResponse>());

    response.reset(new HttpResponse);
    response->setVersion(HttpVersion::kHttp11);
    response->setStatusCode(statusCode);
    response->setHeader("Content-Type", "text/html;charset=utf-8");
    // FIXME 改为动态获取程序名和版本号
    response->setHeader("Server", "tinyserver/1.2.1");

    std::string msg(std::to_string(static_cast<int>(response->statusCode())) + " " + response->statusMessage());
    response->setBody("<html><head><title>" + msg + "</title></head><body><center><h1>" + msg + "</h1></center><hr><center>tinyserver/1.2.1</center></body></html>");

    response->setHeader("Content-Length", std::to_string(response->body().size()));
    // FIXME close or keep-alive?
    response->setHeader("Connection", "close");

    generalResponse_.insert({statusCode, response});

    return response;
}

HttpContext::HttpResponsePtr HttpContext::simpleResponse(HttpVersion version, HttpStatusCode statusCode, const std::string & title, const std::string & content) {
    HttpResponsePtr response(std::make_shared<HttpResponse>());

    response.reset(new HttpResponse);
    response->setVersion(version);
    response->setStatusCode(statusCode);
    response->setHeader("Content-Type", "text/html;charset=utf-8");
    // FIXME 改为动态获取程序名和版本号
    response->setHeader("Server", "tinyserver/1.2.1");

    response->setBody("<html><head><title>" + title + "</title></head><body><p>" + content + "</p><hr><center>tinyserver/1.2.1</center></body></html>");

    response->setHeader("Content-Length", std::to_string(response->body().size()));

    return response;
}

const std::unordered_map<HttpContext::HttpStatusCode, std::string> HttpContext::statusMessage_ {
    {HttpStatusCode::kOk,                       "OK"                        },
    {HttpStatusCode::kBadRequest,               "Bad Request"               },
    {HttpStatusCode::kForbidden,                "Forbidden"                 },
    {HttpStatusCode::kNotFound,                 "Not Found"                 },
    {HttpStatusCode::kMethodNotAllowed,         "Method Not Allowed"        },
    {HttpStatusCode::kInternalServerError,      "Internal Server Error"     },
    {HttpStatusCode::kNotImplemented,           "Not Implemented"           },
    {HttpStatusCode::kHttpVersionNotSupported,  "HTTP Version Not Supported"}
};

const std::unordered_map<HttpContext::HttpVersion, std::string> HttpContext::versionMessage_ {
    {HttpVersion::kHttp10,  "HTTP/1.0"},
    {HttpVersion::kHttp11,  "HTTP/1.1"}
};

const std::unordered_map<HttpContext::HttpMethod, std::string> HttpContext::methodMessage_ {
    {HttpMethod::kGet,  "GET"},
    {HttpMethod::kPost, "POST"}
};

std::unordered_map<HttpContext::HttpStatusCode, HttpContext::HttpResponsePtr> HttpContext::generalResponse_ {};

const std::string HttpContext::crlf {"\r\n"};
const std::string HttpContext::space {" "};
const std::string HttpContext::colon {": "};
