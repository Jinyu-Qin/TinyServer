#include "Socket.h"
#include <stdexcept>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

ServerSocket::ServerSocket()
    : sockfd_(socket(PF_INET, SOCK_STREAM, 0))
    , opened_(false) {
    if(sockfd_ == -1) {
        throw std::runtime_error("couldn't create socket");
    }
}

ServerSocket::~ServerSocket() {
    close();
}

void ServerSocket::setReuseAddress(bool enable) {
    int opt = enable ? 1 : 0;
    int ret = setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, static_cast<void *>(&opt), sizeof(opt));
    if(ret == -1) {
        throw std::runtime_error("couldn't enable/disable SO_REUSEADDR");
    }
}

void ServerSocket::setNonblocking(bool enable) {
    int flag = fcntl(sockfd_, F_GETFL);
    if(flag ==-1) {
        throw std::runtime_error("couldn't get socket flag");
    }
    flag = enable ? (flag | O_NONBLOCK) : (flag & ~O_NONBLOCK);
    int ret = fcntl(sockfd_, F_SETFL, flag);
    if(ret == -1) {
        throw std::runtime_error("couldn't set socket flag");
    }
}

void ServerSocket::bind(const char * addr, int port) {
    struct sockaddr_in serveraddr = {};

    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, addr, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(port);

    int ret = ::bind(sockfd_, reinterpret_cast<struct sockaddr *>(&serveraddr), sizeof(serveraddr));
    if(ret == -1) {
        throw std::runtime_error("couldn't bind address/port");
    }
}

void ServerSocket::listen(int backlog) {
    int ret = ::listen(sockfd_, backlog);
    if(ret == -1) {
        throw std::runtime_error("couldn't listen on socket");
    }
}

int ServerSocket::accept() {
    struct sockaddr_in clientaddr = {};
    socklen_t len = 0;

    int ret = ::accept(sockfd_, reinterpret_cast<struct sockaddr *>(&clientaddr), &len);
    if(ret == -1) {
        throw std::runtime_error("couldn't accept new connection");
    }
}

void ServerSocket::close() {
    if(opened_) {
        opened_ = false;
        int ret = ::close(sockfd_);
        if(ret == -1) {
            throw std::runtime_error("couldn't close socket");
        }
    }
}

bool ServerSocket::closed() const {
    return !opened_;
}

int ServerSocket::fd() const {
    return sockfd_;
}