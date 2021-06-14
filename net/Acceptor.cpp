#include "Acceptor.h"
#include "Socket.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Channel.h"
#include "TimeStamp.h"
#include <sys/socket.h>
#include <glog/logging.h>
#include <errno.h>
#include <cstring>

Acceptor::Acceptor(EventLoop * loop, const InetAddress & localAddr)
    : loop_(loop)
    , listenning_(false) {
    int sockfd = ::socket(PF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        LOG(FATAL) << "Something wrong when call socket() in Acceptor::Acceptor(EventLoop * loop, const InetAddress & localAddr), the errno is " << errno << "(" << strerror(errno) << ")";
    }

    socket_.reset(new Socket(sockfd));
    socket_->setReuseAddr(true);
    socket_->setReusePort(true);
    socket_->setNonBlocking(true);
    socket_->bind(localAddr);

    channel_.reset(new Channel(loop_, socket_->fd()));
    channel_->setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    channel_->disableReading();
    channel_->remove();
}

void Acceptor::setNewConnectionCallback(NewConnectionCallback callback) {
    newConnectionCallback_ = callback;
}

void Acceptor::listen() {
    loop_->assertInLoopThread();

    socket_->listen();
    listenning_ = true;
    channel_->enableReading();
}

bool Acceptor::listenning() const {
    return listenning_;
}

void Acceptor::handleRead() {
    loop_->assertInLoopThread();
    InetAddress peerAddr(sockaddr_in{});
    int ret = socket_->accept(peerAddr);
    if(ret != -1 && newConnectionCallback_) {
        newConnectionCallback_(ret, peerAddr);
    }
}
