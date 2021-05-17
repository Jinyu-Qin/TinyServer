#include "Socket.h"
#include "InetAddress.h"
#include <unistd.h>
#include <cstring>
#include <glog/logging.h>
#include <netinet/tcp.h>
#include <fcntl.h>

Socket::Socket(int sockfd)
    : fd_(sockfd) {
}

Socket::~Socket() {
    int ret = ::close(fd_);
    if(ret == -1) {
        LOG(FATAL) << "Something wrong when call close() in Socket::~Socket(), the errno is " << errno << "(" << strerror(errno) << ")";
    }
}

int Socket::fd() const {
    return fd_;
}

void Socket::bind(const InetAddress & localAddr) {
    struct sockaddr_in addr = static_cast<struct sockaddr_in>(localAddr);
    int ret = ::bind(fd_, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr));
    if(ret == -1) {
        LOG(FATAL) << "Something wrong when call bind() in Socket::bind(const InetAddress & localAddr), the errno is " << errno << "(" << strerror(errno) << ")";
    }
}

void Socket::listen() {
    int ret = ::listen(fd_, SOMAXCONN);
    if(ret == -1) {
        LOG(FATAL) << "Something wrong when call listen() in Socket::listen(), the errno is " << errno << "(" << strerror(errno) << ")";
    }
}

int Socket::accept(InetAddress & peerAddr) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int ret = ::accept4(fd_, reinterpret_cast<struct sockaddr *>(&addr), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(ret == -1) {
        LOG(FATAL) << "Something wrong when call accept4() in Socket::accept(InetAddress & peerAddr), the errno is " << errno << "(" << strerror(errno) << ")";
    }
    peerAddr = addr;
    return ret;
}

void Socket::shutdownWrite() {
    int ret = ::shutdown(fd_, SHUT_WR);
    if(ret == -1) {
        LOG(FATAL) << "Something wrong when call shutdown() in Socket::shutdownWrite(), the errno is " << errno << "(" << strerror(errno) << ")";
    }
}

void Socket::setReuseAddr(bool enabled) {
    int opt = enabled ? 1 : 0;
    int ret = ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
    if(ret == -1) {
        LOG(FATAL) << "Something wrong when call setsockopt() in Socket::setReuseAddr(bool enabled), the errno is " << errno << "(" << strerror(errno) << ")";
    }
}

void Socket::setReusePort(bool enabled) {
    int opt = enabled ? 1 : 0;
    int ret = ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(int));
    if(ret == -1) {
        LOG(FATAL) << "Something wrong when call setsockopt() in Socket::setReusePort(bool enabled), the errno is " << errno << "(" << strerror(errno) << ")";
    }
}

void Socket::setKeepAlive(bool enabled) {
    int opt = enabled ? 1 : 0;
    int ret = ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(int));
    if(ret == -1) {
        LOG(FATAL) << "Something wrong when call setsockopt() in Socket::setKeepAlive(bool enabled), the errno is " << errno << "(" << strerror(errno) << ")";
    }
}

void Socket::setTcpNoDelay(bool enabled) {
    int opt = enabled ? 1 : 0;
    int ret = ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(int));
    if(ret == -1) {
        LOG(FATAL) << "Something wrong when call setsockopt() in Socket::setTcpNoDelay(bool enabled), the errno is " << errno << "(" << strerror(errno) << ")";
    }
}

void Socket::setNonBlocking(bool enabled) {
    int opt = ::fcntl(fd_, F_GETFL);
    opt = enabled ? opt | O_NONBLOCK : opt & ~O_NONBLOCK;
    int ret = ::fcntl(fd_, F_SETFL, opt);
    if(ret == -1) {
        LOG(FATAL) << "Something wrong when call fcntl() in Socket::setNonBlocking(bool enabled), the errno is " << errno << "(" << strerror(errno) << ")";
    }
}