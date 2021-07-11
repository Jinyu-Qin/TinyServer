#ifndef __HTTPSERVICE_H__
#define __HTTPSERVICE_H__

#include <boost/utility.hpp>
#include <memory>
#include <string>
#include "HttpContext.h"

class HttpRequest;
class HttpResponse;

class HttpService: public boost::noncopyable {
public:
    using HttpMethod        = HttpContext::HttpMethod;
    using HttpVersion       = HttpContext::HttpVersion;
    using HttpStatusCode    = HttpContext::HttpStatusCode;
    using HttpRequestPtr    = std::shared_ptr<HttpRequest>;
    using HttpResponsePtr   = std::shared_ptr<HttpResponse>;

    HttpService(const std::string & root);
    ~HttpService();

    void service(HttpRequestPtr request, HttpResponsePtr response);

private:
    void doGet(HttpRequestPtr request, HttpResponsePtr response);
    void doPost(HttpRequestPtr request, HttpResponsePtr response);

    HttpResponsePtr executeCgi(HttpRequestPtr request);

    const std::string root_;
};

#endif //__HTTPSERVICE_H__
