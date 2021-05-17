#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include <boost/utility.hpp>
#include <memory>
#include <functional>

class EventLoop;
class Channel;
class Socket;
class InetAddress;

class Acceptor: public boost::noncopyable {
public:
    using NewConnectionCallback = std::function<void(int, const InetAddress  &)>;

    Acceptor(EventLoop * loop, const InetAddress & localAddr);
    ~Acceptor();

    // 设置新连接回调函数
    void setNewConnectionCallback(NewConnectionCallback callback);
    // 开始监听
    void listen();
    // 监听
    bool listenning() const;

private:
    void handleRead();

    EventLoop * loop_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    bool listenning_;

    NewConnectionCallback newConnectionCallback_;
};

#endif //__ACCEPTOR_H__
