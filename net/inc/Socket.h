#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <boost/utility.hpp>

class InetAddress;

class Socket: public boost::noncopyable {
public:
    explicit Socket(int sockfd);
    ~Socket();

    // 获取文件描述符
    int fd() const;
    // 绑定本地IP地址
    void bind(const InetAddress & localAddr);
    // 监听端口
    void listen();
    // 接收新的连接
    int accept(InetAddress & peerAddr);

    // 半关闭TCP连接
    void shutdownWrite();
    // 设置地址重用
    void setReuseAddr(bool enabled);
    // 设置端口重用
    void setReusePort(bool enabled);
    // 设置TCP连接保活
    void setKeepAlive(bool enabled);
    // 设置是否禁用negle算法
    void setTcpNoDelay(bool enabled);
    // 设置非阻塞
    void setNonBlocking(bool enabled);

private:
    int fd_;
};

#endif //__SOCKET_H__
