#ifndef __ECHOSERVER_H__
#define __ECHOSERVER_H__

#include <boost/utility.hpp>
#include <memory>
#include <string>
#include "InetAddress.h"

class EventLoop;
class TcpServer;
class TcpConnection;
class Buffer;
class TimeStamp;

class EchoServer: public boost::noncopyable {
public:
    using TcpConnectionPtr  = std::shared_ptr<TcpConnection>;
    using BufferPtr         = Buffer *;

    EchoServer(EventLoop * loop, const std::string & name, const InetAddress & localAddr);
    ~EchoServer();

    EventLoop * getLoop() const;
    const std::string & name() const;
    const InetAddress & localAddress() const;

    void start(int numThread = 4);
    void stop();

private:
    void handleConnection(TcpConnectionPtr conn);
    void handleMessage(TcpConnectionPtr conn, BufferPtr message, TimeStamp receiveTime);

    EventLoop * loop_;
    const std::string name_;
    const InetAddress localAddr_;
    std::unique_ptr<TcpServer> tcpServer_;

    bool started_;
};

#endif //__ECHOSERVER_H__
