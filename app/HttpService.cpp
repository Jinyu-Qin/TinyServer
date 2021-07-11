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

        if(st.st_mode & S_IXUSR) {
            resp = executeCgi(request);
        } else {
            ssize_t nBytes = ::read(fd, msg.data(), msg.size());
            if(nBytes == -1) {
                resp = HttpContext::generalResponse(HttpStatusCode::kInternalServerError);
            } else {
                resp = HttpContext::simpleResponse(request->version(), HttpStatusCode::kOk, request->path(), std::string(msg.data(), nBytes));
            }
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

void HttpService::doPost(HttpRequestPtr request, HttpResponsePtr response) {// 计算文件地址
    HttpResponsePtr resp;

    std::string realPath((root_.back() == '/' ? root_.substr(0, root_.size()) : root_) + request->path());
    struct stat st;
    bzero(&st, sizeof(st));
    if(stat(realPath.c_str(), &st) == -1) {
        resp = HttpContext::generalResponse(HttpStatusCode::kNotFound);
    } else if(st.st_mode & S_IXUSR) {
        resp = executeCgi(request);
    } else {
        resp = HttpContext::generalResponse(HttpContext::kForbidden);
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

HttpService::HttpResponsePtr HttpService::executeCgi(HttpRequestPtr request) {
    std::string realPath((root_.back() == '/' ? root_.substr(0, root_.size()) : root_) + request->path());
    int cgiInput[2] = {};
    int cgiOutput[2] = {};
    pipe(cgiInput);
    pipe(cgiOutput);

    pid_t pid = fork();
    if(pid == -1) {
        close(cgiInput[0]);
        close(cgiInput[1]);
        close(cgiOutput[0]);
        close(cgiOutput[1]);
        return HttpContext::generalResponse(HttpStatusCode::kInternalServerError);
    } else if(pid == 0) {
        dup2(cgiOutput[1], STDOUT_FILENO);
        dup2(cgiInput[0], STDIN_FILENO);
        close(cgiOutput[0]);
        close(cgiInput[1]);

        char methodEnv[256];
        sprintf(methodEnv, "REQUEST_METHOD=%s", HttpContext::getMethodMessage(request->method()).c_str());
        putenv(methodEnv);

        if(request->method() == HttpMethod::kGet) {
            char queryEnv[512];
            sprintf(queryEnv, "QUERY_STRING=%s", request->query().c_str());
            putenv(queryEnv);
        } else if(request->method() == HttpMethod::kPost) {
            char contentLengthEnv[128];
            sprintf(contentLengthEnv, "CONTENT_LENGTH=%d", request->getHeader("Content-Length"));
            putenv(contentLengthEnv);
        }

        execl(realPath.c_str(), NULL);
        
        exit(0);
    } else {
        close(cgiOutput[1]);
        close(cgiInput[0]);

        if(request->method() == HttpMethod::kPost) {
            write(cgiInput[1], request->body().c_str(), request->body().size());
        }

        char buf[1024] = {0};
        int nBytes = 0;
        std::string msg;
        while((nBytes = read(cgiOutput[0], &buf, sizeof(buf) - 1)) != 0) {
            if(nBytes < 0) {
                close(cgiInput[1]);
                close(cgiOutput[0]);
                return HttpContext::generalResponse(HttpStatusCode::kInternalServerError);
            } else {
                msg.append(buf);
            }
        }

        close(cgiInput[1]);
        close(cgiOutput[0]);
        return HttpContext::simpleResponse(HttpVersion::kHttp11, HttpStatusCode::kOk, request->path(), msg);
    }

    return HttpContext::generalResponse(HttpStatusCode::kInternalServerError);
}