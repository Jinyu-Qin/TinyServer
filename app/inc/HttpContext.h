#ifndef __HTTPCONTEXT_H__
#define __HTTPCONTEXT_H__

#include <boost/utility.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <functional>

class Buffer;
class HttpRequest;
class HttpResponse;
class TcpConnection;
class TimeStamp;

class HttpContext: public boost::noncopyable {
public:
    using BufferPtr         = Buffer *;
    using HttpRequestPtr    = std::shared_ptr<HttpRequest>;
    using HttpResponsePtr   = std::shared_ptr<HttpResponse>;
    using TcpConnectionPtr  = std::shared_ptr<TcpConnection>;
    using ServiceCallback   = std::function<void(HttpRequestPtr, HttpResponsePtr)>;

    enum HttpMethod {
        kInvalid,
        kGet,
        kPost
    };

    enum HttpVersion {
        kUnknown,
        kHttp10,
        kHttp11
    };

    enum HttpStatusCode {
        kOk                         = 200,
        kBadRequest                 = 400,
        kForbidden                  = 403,
        kNotFound                   = 404,
        kMethodNotAllowed           = 405,
        kInternalServerError        = 500,
        kNotImplemented             = 501,
        kHttpVersionNotSupported    = 505
    };

    HttpContext(TcpConnectionPtr conn);
    ~HttpContext();

    TcpConnectionPtr getTcpConnection() const;

    void process(BufferPtr message, TimeStamp received);
    void setServiceCallback(ServiceCallback callback);

    static const std::string & getStatusMessage(HttpStatusCode statusCode);
    static const std::string & getVersionMessage(HttpVersion version);
    static const std::string & getMethodMessage(HttpMethod method);
    static HttpResponsePtr generalResponse(HttpStatusCode statusCode);
    static HttpResponsePtr simpleResponse(HttpVersion version, HttpStatusCode statusCode, const std::string & title, const std::string & content);

private:
    enum HttpRequestDecodeState {
        kDecodeRequestLine,
        kDecodeRequestHeaders,
        kDecodeRequestBody,
        kDecodeRequestDone,
        kDecodeRequestError
    };

    const char * findCRLF(BufferPtr message);
    const char * findSpace(BufferPtr message);
    const char * findColon(BufferPtr message);
    const char * findQuestionMark(BufferPtr message);
    void decodeHttpRequest(HttpRequestPtr request, BufferPtr message);
    void encodeHttpResponse(HttpResponsePtr response, BufferPtr message);
    void handleRequestError();
    void handleProcessError();

    TcpConnectionPtr conn_;
    ServiceCallback serviceCallback_;

    HttpRequestPtr request_;
    HttpResponsePtr response_;

    HttpRequestDecodeState requestDecodeState_;
    ssize_t requestBodyRemainingSize_;
    const char * requestBodyEnd_;

    std::unique_ptr<Buffer> responseBuffer_;

    static const std::unordered_map<HttpStatusCode, std::string> statusMessage_;
    static const std::unordered_map<HttpVersion, std::string> versionMessage_;
    static const std::unordered_map<HttpMethod, std::string> methodMessage_;
    static std::unordered_map<HttpStatusCode, HttpResponsePtr> generalResponse_;

    static const std::string crlf;
    static const std::string space;
    static const std::string colon;
};

#endif //__HTTPCONTEXT_H__
