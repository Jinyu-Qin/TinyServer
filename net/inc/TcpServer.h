#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__

#include <boost/utility.hpp>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include "InetAddress.h"

class Acceptor;
class EventLoop;
class EventLoopThreadPool;
class TcpConnection;
class Buffer;
class TimeStamp;

class TcpServer: public boost::noncopyable {
public:
    using BufferPtr             = Buffer *;
    using TcpConnectionPtr      = std::shared_ptr<TcpConnection>;
    
    using ConnectionCallback    = std::function<void(TcpConnectionPtr)>;
    using WriteCompleteCallback = std::function<void(TcpConnectionPtr)>;
    using MessageCallback       = std::function<void(TcpConnectionPtr, BufferPtr, TimeStamp)>;
    using ThreadInitCallback    = std::function<void(EventLoop *)>;

    TcpServer(EventLoop * loop, const InetAddress & localAddr, const std::string & name);
    ~TcpServer();

    // 获取本地地址
    const InetAddress & localAddress() const;
    // 获取名称
    const std::string & name() const;
    // 获取所属的EventLoop
    EventLoop * getLoop() const;

    // 设置线程数
    void setThreadNum(int numThreads);
    // 设置线程初始化回调函数
    void setThreadInitCallback(ThreadInitCallback callback);
    // 启动服务器
    void start();

    // 设置连接回调函数
    void setConnectionCallback(ConnectionCallback callback);
    // 设置接收完毕回调函数
    void setMessageCallback(MessageCallback callback);
    // 设置发送完毕回调函数
    void setWriteCompleteCallback(WriteCompleteCallback callback);

private:
    void handleNewConnection(int sockfd, const InetAddress & peerAddr);
    void handleRemoveConnection(TcpConnectionPtr conn);
    void removeConnectionInLoop(TcpConnectionPtr conn);

    EventLoop * loop_;
    const InetAddress localAddr_;
    const std::string name_;
    
    std::unique_ptr<Acceptor> acceptor_;
    std::unique_ptr<EventLoopThreadPool> threadPool_;
    ThreadInitCallback threadInitCallback_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    bool started_;
    int nextConnId_;
    std::unordered_map<std::string, TcpConnectionPtr> connections_;
};


#endif //__TCPSERVER_H__
