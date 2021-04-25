#ifndef __ECHOSERVER_H__
#define __ECHOSERVER_H__

#include <boost/utility.hpp>
#include "TcpServer.h"
#include "Logger.h"

class EchoServer: public boost::noncopyable {
    using TcpServerPtr = std::unique_ptr<TcpServer>;
public:
    using TcpConnectionPtr = TcpConnection::TcpConnectionPtr;
    using BufferPtr = TcpConnection::BufferPtr;

    explicit EchoServer(EventLoop * loop, const std::string & addr = std::string("0.0.0.0"), int port = 2021);
    ~EchoServer();

    void start(int numThread = 4);

private:
    void onNewConnection(TcpConnectionPtr conn);
    void onMessageReceived(TcpConnectionPtr conn, BufferPtr buffer);
    void onMessageSent(TcpConnectionPtr conn);
    void onConnectionClosed(TcpConnectionPtr conn);

    EventLoop * loop_;
    TcpServerPtr tcpServer_;
    Logger::LoggerPtr logger_;
};

#endif //__ECHOSERVER_H__
