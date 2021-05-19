#include "HttpResponse.h"
#include <algorithm>
#include <cctype>

HttpResponse::HttpResponse()
    : version_(HttpVersion::kUnknown)
    , statusCode_(HttpStatusCode::kInternalServerError) {
}

HttpResponse::~HttpResponse() {
}

HttpResponse::HttpVersion HttpResponse::version() const {
    return version_;
}

void HttpResponse::setVersion(HttpVersion version) {
    version_ = version;
}

HttpResponse::HttpStatusCode HttpResponse::statusCode() const {
    return statusCode_;
}

void HttpResponse::setStatusCode(HttpStatusCode statusCode) {
    statusCode_ = statusCode;
}

const std::string & HttpResponse::statusMessage() const {
    return HttpContext::getStatusMessage(statusCode_);
}

const std::string & HttpResponse::body() const {
    return body_;
}

void HttpResponse::setBody(const std::string & body) {
    body_ = body;
}

const std::string & HttpResponse::getHeader(const std::string & key) const {
    auto it = headers_.find(key);
    return it == headers_.cend() ? null : it->second;
}

void HttpResponse::setHeader(const std::string & key, const std::string & value) {
    headers_[key] = value;
}

const std::unordered_map<std::string, std::string> & HttpResponse::headers() const {
    return headers_;
}

const std::string HttpResponse::null {};
