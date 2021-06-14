#ifndef __TCPCONNECTION_H__
#define __TCPCONNECTION_H__

#include <boost/utility.hpp>
#include <boost/any.hpp>
#include <memory>
#include <string>
#include <functional>
#include "InetAddress.h"
#include "Buffer.h"

class EventLoop;
class Channel;
class Socket;
class TimeStamp;

class TcpConnection: public boost::noncopyable, public std::enable_shared_from_this<TcpConnection> {
public:
    using BufferPtr             = Buffer *;
    using TcpConnectionPtr      = std::shared_ptr<TcpConnection>;
    
    using ConnectionCallback    = std::function<void(TcpConnectionPtr)>;
    using CloseCallback         = std::function<void(TcpConnectionPtr)>;
    using WriteCompleteCallback = std::function<void(TcpConnectionPtr)>;
    using MessageCallback       = std::function<void(TcpConnectionPtr, BufferPtr, TimeStamp)>;

    TcpConnection(EventLoop * loop, const std::string & name, int sockfd, const InetAddress & localAddr, const InetAddress & peerAddr);
    ~TcpConnection();

    // 获取TcpConnection所绑定的EventLoop
    EventLoop * getLoop() const;
    // 获取TcpConnection的名称
    const std::string & name() const;
    // 获取本地地址
    const InetAddress & localAddress() const;
    // 获取对端地址
    const InetAddress & peerAddress() const;
    // 获取hash值
    uint64_t hashCode() const;
    // 是否已建立连接
    bool connected() const;
    // 是否已断开连接
    bool disconnected() const;

    // 发送数据
    void send(const void * message, size_t size);
    // 发送数据
    void send(BufferPtr message);

    // 半关闭
    void shutdown();
    // 全关闭
    void forceClose();

    // 开始读
    void startRead();
    // 停止读
    void stopRead();
    // 是否正在读
    bool isReading() const;

    // 设置连接回调函数（建立连接和断开连接时都会调用）
    void setConnectionCallback(ConnectionCallback callback);
    // 设置接收完消息回调函数
    void setMessageCallback(MessageCallback callback);
    // 设置发送完消息回调函数
    void setWriteCompleteCallback(WriteCompleteCallback callback);
    // 设置连接关闭回调函数（仅供TcpServer调用）
    void setCloseCallback(CloseCallback callback);

    // 连接建立时调用（仅供TcpServer调用一次）
    void connectEstablished();
    // 连接断开时调用（仅供TcpServer调用一次）
    void connectDestroyed();

    // 设置TCP连接的上下文
    void setContext(const boost::any & context);
    // 获取TCP连接的上下文
    boost::any & getContext();

private:
    enum TcpConnectionState {
        kConnecting,
        kConnected,
        kDisconnecting,
        kDisconnected
    };

    void handleRead(TimeStamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void * message, size_t size);
    void sendInLoop(std::shared_ptr<Buffer> message);
    void shutdownInLoop();
    void forceCloseInLoop();
    void startReadInLoop();
    void stopReadInLoop();

    static const std::string & stateString(TcpConnectionState state);

    EventLoop * loop_;                  // TcpConnection所绑定的EventLoop对象
    const std::string name_;            // TcpConnection名称
    const InetAddress localAddr_;       // 本地地址
    const InetAddress peerAddr_;        // 对端地址

    TcpConnectionState state_;          // TcpConnection所处状态
    bool reading_;                      // 标识是否正在读
    std::unique_ptr<Socket> socket_;    // TcpConnection持有的Socket对象
    std::unique_ptr<Channel> channel_;  // TcpConnectione持有的Channel对象

    ConnectionCallback connectionCallback_;         // 连接回调函数
    MessageCallback messageCallback_;               // 读完成回调函数
    WriteCompleteCallback writeCompleteCallback_;   // 写完成回调函数
    CloseCallback closeCallback_;                   // 连接关闭回调函数

    Buffer inputBuffer_;                // 接收缓冲
    Buffer outputBuffer_;               // 发送缓冲

    boost::any context_;                // TCP连接的上下文对象（留给应用层使用）

    static const std::string stateStr[];
};


#endif //__TCPCONNECTION_H__
