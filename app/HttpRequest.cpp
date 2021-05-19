#include "HttpRequest.h"
#include <cassert>

HttpRequest::HttpRequest()
    : method_(HttpMethod::kInvalid)
    , version_(HttpVersion::kUnknown) {
}

HttpRequest::~HttpRequest() {
}

HttpRequest::HttpMethod HttpRequest::method() const {
    return method_;
}

void HttpRequest::setMethod(HttpMethod method) {
    method_ = method;
}

void HttpRequest::setMethod(const std::string & methodStr) {
    auto it = methodMap_.find(methodStr);
    setMethod(it == methodMap_.cend() ? HttpMethod::kInvalid : it->second);
}

void HttpRequest::setMethod(const char * begin, const char * end) {
    return setMethod(std::string(begin, end));
}

HttpRequest::HttpVersion HttpRequest::version() const {
    return version_;
}

void HttpRequest::setVersion(HttpVersion version) {
    version_ = version;
}

void HttpRequest::setVersion(const std::string & versionStr) {
    auto it = versionMap_.find(versionStr);
    setVersion(it == versionMap_.cend() ? HttpVersion::kUnknown : it->second);
}

void HttpRequest::setVersion(const char * begin, const char * end) {
    return setVersion(std::string(begin, end));
}

const std::string & HttpRequest::path() const {
    return path_;
}

void HttpRequest::setPath(const std::string & path) {
    path_ = path;
}

void HttpRequest::setPath(const char * begin, const char * end) {
    setPath(std::string(begin, end));
}

const std::string & HttpRequest::query() const {
    assert(method_ == HttpMethod::kGet);
    return query_;
}

void HttpRequest::setQuery(const std::string & query) {
    assert(method_ == HttpMethod::kGet);
    query_ = query;
}

void HttpRequest::setQuery(const char * begin, const char * end) {
    assert(method_ == HttpMethod::kGet);
    setQuery(std::string(begin, end));
}

const std::string & HttpRequest::body() const {
    assert(method_ == HttpMethod::kPost);
    return query_;
}

void HttpRequest::setBody(const std::string & body) {
    assert(method_ == HttpMethod::kPost);
    query_ = body;
}

void HttpRequest::setBody(const char * begin, const char * end) {
    assert(method_ == HttpMethod::kPost);
    setBody(std::string(begin, end));
}

void HttpRequest::appendBody(const std::string & body) {
    assert(method_ == HttpMethod::kPost);
    query_.append(body);
}

void HttpRequest::appendBody(const char * begin, const char * end) {
    assert(method_ == HttpMethod::kPost);
    appendBody(std::string(begin, end));
}

const std::string & HttpRequest::getHeader(const std::string & key) const {
    auto it = headers_.find(key);
    return it == headers_.cend() ? null : it->second;
}

void HttpRequest::setHeader(const std::string & key, const std::string & value) {
    headers_[key] = value;
}

void HttpRequest::setHeader(const std::string & keyValueStr) {
    size_t split = keyValueStr.find(": ");
    assert(split != std::string::npos);

    setHeader(keyValueStr.substr(0, split), keyValueStr.substr(split + 2));
}

void HttpRequest::setHeader(const char * begin, const char * end) {
    setHeader(std::string(begin, end));
}

const std::unordered_map<std::string, std::string> & HttpRequest::headers() const {
    return headers_;
}

const std::string HttpRequest::null {};

const std::unordered_map<std::string, HttpRequest::HttpMethod> HttpRequest::methodMap_ {
    {"kGet", HttpMethod::kGet},
    {"kPost", HttpMethod::kPost},

    {"GET", HttpMethod::kGet},
    {"POST", HttpMethod::kPost}
};

const std::unordered_map<std::string, HttpRequest::HttpVersion> HttpRequest::versionMap_ {
    {"kHttp10", HttpVersion::kHttp10},
    {"kHttp11", HttpVersion::kHttp11},

    {"HTTP/1.0", HttpVersion::kHttp10},
    {"HTTP/1.1", HttpVersion::kHttp11}
};
