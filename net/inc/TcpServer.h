#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__

#include <boost/utility.hpp>
#include <functional>
#include <vector>
#include "EventLoop.h"
#include "TcpConnection.h"
#include "EventLoopThreadPool.h"
#include "Socket.h"
#include "Channel.h"

class TcpServer: public boost::noncopyable {
    using EventLoopThreadPoolPtr = std::unique_ptr<EventLoopThreadPool>;
    using ServerSocketPtr = std::unique_ptr<ServerSocket>;
    using ChannelPtr = std::unique_ptr<Channel>;
public:
    using TcpConnectionPtr = typename TcpConnection::TcpConnectionPtr;
    using BufferPtr = typename TcpConnection::BufferPtr;

    using NewConnectionCallback = std::function<void(TcpConnectionPtr)>;
    using MessageReceivedCallback = typename TcpConnection::MessageReceivedCallback;
    using MessageSentCallback = typename TcpConnection::MessageSentCallback;
    using ConnectionClosedCallback = typename TcpConnection::CloseCallback;
    using ConnectionErrorCallback = typename TcpConnection::ErrorCallback;
    using ThreadInitCallback = typename EventLoopThreadPool::ThreadInitCallback;

    explicit TcpServer(EventLoop * loop, const std::string & addr = std::string("0.0.0.0"), int port = 8080, int maxConn = 1024, const std::string & name = std::string());
    ~TcpServer();

    void setNewConnectionCallback(NewConnectionCallback callback);
    void setMessageReceivedCallback(MessageReceivedCallback callback);
    void setMessageSentCallback(MessageSentCallback callback);
    void setConnectionClosedCallback(ConnectionClosedCallback callback);
    void setConnectionErrorCallback(ConnectionErrorCallback callback);
    void setThreadInitCallback(ThreadInitCallback callback);

    void start(int numThread = 4);

    const std::string & serverAddress() const;
    int serverPort() const;
    const std::string & name() const;

private:
    void handleNewConnection();
    void handleRemoveConnection(TcpConnectionPtr conn);
    void handleError();

    void setNonblocking(int fd);

    EventLoop * loop_;
    std::string address_;
    int port_;
    const int maxConnection_;
    int numConnection_;
    std::string name_;

    EventLoopThreadPoolPtr pool_;
    ServerSocketPtr serverSocket_;
    ChannelPtr serverSocketChannel_;
    std::vector<TcpConnectionPtr> connections_;

    NewConnectionCallback newConnectionCallback_;
    MessageReceivedCallback messageReceivedCallback_;
    MessageSentCallback messageSentCallback_;
    ConnectionClosedCallback connectionClosedCallback_;
    ConnectionErrorCallback connectionErrorCallback_;
    ThreadInitCallback threadInitCallback_;
};

#endif //__TCPSERVER_H__
