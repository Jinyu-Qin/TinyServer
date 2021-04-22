#ifndef __TCPCONNECTION_H__
#define __TCPCONNECTION_H__

#include <boost/utility.hpp>
#include <functional>
#include <string>
#include <memory>
#include "EventLoop.h"
#include "Buffer.h"

class TcpConnection: public boost::noncopyable, public std::enable_shared_from_this<TcpConnection> {
    using ChannelPtr = std::unique_ptr<Channel>;
    using BufferPtr = Buffer *;

public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using MessageReceivedCallback = std::function<void(TcpConnectionPtr, BufferPtr)>;
    using MessageSentCallback = std::function<void(TcpConnectionPtr)>;
    using CloseCallback = std::function<void(TcpConnectionPtr)>;
    using ErrorCallback = std::function<void(TcpConnectionPtr)>;

    ~TcpConnection();

    void setMessageReceivedCallback(MessageReceivedCallback callback);
    void setMessageSentCallback(MessageSentCallback callback);
    void setCloseCallback(CloseCallback callback);
    void setErrorCallback(ErrorCallback callback);

    bool disconnected() const;
    const std::string & name() const;

    void send(const char * buf, int len);
    void send(const std::string & str);
    void shutdown();

    static TcpConnectionPtr createTcpConnection(EventLoop * loop, int sockfd, const std::string & name = std::string());

private:
    TcpConnection(EventLoop * loop, int sockfd, const std::string & name);

    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();
    
    void sendInLoop(const char * buf, int len);
    void sendInLoop(std::shared_ptr<char> buf, int len);

    void shutdownInLoop();
    
    EventLoop * loop_;
    int sockfd_;
    std::string name_;

    bool disconnected_;
    ChannelPtr channel_;

    MessageReceivedCallback messageReceivedCallback_;
    MessageSentCallback messageSentCallback_;
    CloseCallback closeCallback_;
    ErrorCallback errorCallback_;

    Buffer bufferIn_;
    Buffer bufferOut_;

    static constexpr int kBufferInCapacity_ = 8192;
    static constexpr int kBufferOutCapacity_ = 8192;
};

#endif //__TCPCONNECTION_H__
