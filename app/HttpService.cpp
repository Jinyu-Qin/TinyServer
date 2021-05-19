#include "HttpService.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <glog/logging.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>

HttpService::HttpService(const std::string & root)
    : root_(root) {
}

HttpService::~HttpService() {
}

void HttpService::service(HttpRequestPtr request, HttpResponsePtr response) {
    switch(request->method()) {
        case HttpMethod::kGet:
            doGet(request, response);
        break;

        case HttpMethod::kPost:
            doPost(request, response);
        break;
    }
}

void HttpService::doGet(HttpRequestPtr request, HttpResponsePtr response) {
    // 计算文件地址
    std::string realPath((root_.back() == '/' ? root_.substr(0, root_.size()) : root_) + request->path());
    DLOG(INFO) << "Real Path: " << realpath;

    HttpResponsePtr resp;
    int fd = ::open(realPath.c_str(), O_RDONLY);
    if(fd == -1) {
        if(errno == EACCES) {
            resp = HttpContext::generalResponse(HttpStatusCode::kForbidden);
        } else if(errno == ENOENT) {
            resp = HttpContext::generalResponse(HttpStatusCode::kNotFound);
        } else {
            resp = HttpContext::generalResponse(HttpStatusCode::kInternalServerError);
        }
    } else {
        struct stat st;
        bzero(&st, sizeof(st));
        stat(realPath.c_str(), &st);
        std::vector<char> msg;
        msg.resize(st.st_size);

        ssize_t nBytes = ::read(fd, msg.data(), msg.size());
        if(nBytes == -1) {
            resp = HttpContext::generalResponse(HttpStatusCode::kInternalServerError);
        } else {
            resp = HttpContext::simpleResponse(request->version(), HttpStatusCode::kOk, request->path(), std::string(msg.data(), nBytes));
        }
    }

    // FIXME 性能优化
    response->setVersion(resp->version());
    response->setStatusCode(resp->statusCode());
    for(const auto & kv : resp->headers()) {
        response->setHeader(kv.first, kv.second);
    }
    response->setBody(resp->body());

    const std::string & connectionHeader = request->getHeader("Connection");
    response->setHeader("Connection", connectionHeader.empty() ? (request->version() == HttpVersion::kHttp11 ? "keep-alive" : "close") : connectionHeader);
}

void HttpService::doPost(HttpRequestPtr request, HttpResponsePtr response) {
    HttpResponsePtr resp = HttpContext::generalResponse(HttpStatusCode::kNotImplemented);
    // FIXME 性能优化
    response->setVersion(resp->version());
    response->setStatusCode(resp->statusCode());
    for(const auto & kv : resp->headers()) {
        response->setHeader(kv.first, kv.second);
    }
    response->setBody(resp->body());

    const std::string & connectionHeader = request->getHeader("Connection");
    response->setHeader("Connection", connectionHeader.empty() ? (request->version() == HttpVersion::kHttp11 ? "keep-alive" : "close") : connectionHeader);
}
