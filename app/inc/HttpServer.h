#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__

#include <boost/utility.hpp>
#include <string>
#include <memory>
#include <unordered_map>
#include "InetAddress.h"
#include "Mutex.h"

class EventLoop;
class TcpConnection;
class TimeStamp;
class Buffer;
class HttpService;
class HttpContext;
class HttpRequest;
class HttpResponse;
class TcpServer;

class HttpServer: public boost::noncopyable {
public:
    using TcpConnectionPtr  = std::shared_ptr<TcpConnection>;
    using BufferPtr         = Buffer *;
    using HttpRequestPtr    = std::shared_ptr<HttpRequest>;
    using HttpResponsePtr   = std::shared_ptr<HttpResponse>;
    using HttpContextPtr    = HttpContext *;

    HttpServer(EventLoop * loop, const std::string & name, const InetAddress & localAddr, const std::string & root);
    ~HttpServer();

    EventLoop * getLoop() const;
    const std::string & name() const;
    const InetAddress & localAddress() const;

    void start(int numThreads = 4);
    void stop();

private:
    void handleConnection(TcpConnectionPtr conn);
    void handleMessage(TcpConnectionPtr conn, BufferPtr message, TimeStamp receiveTime);

    EventLoop * loop_;
    const std::string name_;
    const InetAddress localAddr_;
    const std::string root_;
    std::unique_ptr<HttpService> service_;

    bool started_;
    std::unique_ptr<TcpServer> tcpServer_;

    MutexLock mutex_;
};

#endif //__HTTPSERVER_H__
