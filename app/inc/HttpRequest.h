#ifndef __HTTPREQUEST_H__
#define __HTTPREQUEST_H__

#include <boost/utility.hpp>
#include <string>
#include <unordered_map>
#include "HttpContext.h"

class HttpRequest: public boost::noncopyable {
public:
    using HttpMethod    = HttpContext::HttpMethod;
    using HttpVersion   = HttpContext::HttpVersion;

    HttpRequest();
    ~HttpRequest();

    HttpMethod method() const;
    void setMethod(HttpMethod method);
    void setMethod(const std::string & methodStr);
    void setMethod(const char * begin, const char * end);

    HttpVersion version() const;
    void setVersion(HttpVersion version);
    void setVersion(const std::string & versionStr);
    void setVersion(const char * begin, const char * end);

    const std::string & path() const;
    void setPath(const std::string & path);
    void setPath(const char * begin, const char * end);

    const std::string & query() const;
    void setQuery(const std::string & query);
    void setQuery(const char * begin, const char * end);

    const std::string & body() const;
    void setBody(const std::string & body);
    void setBody(const char * begin, const char * end);
    void appendBody(const std::string & body);
    void appendBody(const char * begin, const char * end);

    const std::string & getHeader(const std::string & key) const;
    void setHeader(const std::string & key, const std::string & value);
    void setHeader(const std::string & keyValueStr);
    void setHeader(const char * begin, const char * end);

    const std::unordered_map<std::string, std::string> & headers() const;

private:
    HttpMethod method_;
    HttpVersion version_;
    std::string path_;
    std::string query_;
    std::unordered_map<std::string, std::string> headers_;

    static const std::string null;
    static const std::unordered_map<std::string, HttpMethod> methodMap_;
    static const std::unordered_map<std::string, HttpVersion> versionMap_;
};

#endif //__HTTPREQUEST_H__
