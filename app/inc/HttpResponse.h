#ifndef __HTTPRESPONSE_H__
#define __HTTPRESPONSE_H__

#include <boost/utility.hpp>
#include <string>
#include <unordered_map>
#include "HttpContext.h"

class HttpResponse: public boost::noncopyable {
public:
    using HttpVersion       = HttpContext::HttpVersion;
    using HttpStatusCode    = HttpContext::HttpStatusCode;

    HttpResponse();
    ~HttpResponse();

    HttpVersion version() const;
    void setVersion(HttpVersion version);

    HttpStatusCode statusCode() const;
    void setStatusCode(HttpStatusCode statusCode);

    const std::string & statusMessage() const;

    const std::string & body() const;
    void setBody(const std::string & body);

    const std::string & getHeader(const std::string & key) const;
    void setHeader(const std::string & key, const std::string & value);
    
    const std::unordered_map<std::string, std::string> & headers() const;
private:
    HttpVersion version_;
    HttpStatusCode statusCode_;
    std::string body_;
    std::unordered_map<std::string, std::string> headers_;

    static const std::string null;
};

#endif //__HTTPRESPONSE_H__
