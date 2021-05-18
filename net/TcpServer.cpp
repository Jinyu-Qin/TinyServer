#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "TcpConnection.h"
#include "EventLoop.h"
#include <cassert>
#include <glog/logging.h>

TcpServer::TcpServer(EventLoop * loop, const InetAddress & localAddr, const std::string & name)
    : loop_(loop)
    , localAddr_(localAddr)
    , name_(name)
    , acceptor_(new Acceptor(loop, localAddr_))
    , threadPool_(new EventLoopThreadPool(loop, name_))
    , started_(false)
    , nextConnId_(0) {
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::handleNewConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    for(auto & item : connections_) {
        auto conn = item.second;
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

const InetAddress & TcpServer::localAddress() const {
    return localAddr_;
}

const std::string & TcpServer::name() const {
    return name_;
}

EventLoop * TcpServer::getLoop() const {
    return loop_;
}

void TcpServer::setThreadNum(int numThreads) {
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
    assert(!started_);
    assert(!acceptor_->listenning());

    threadPool_->start(threadInitCallback_);
    acceptor_->listen();
    started_ = true;
}

void TcpServer::setThreadInitCallback(ThreadInitCallback callback) {
    threadInitCallback_ = callback;
}

void TcpServer::setConnectionCallback(ConnectionCallback callback) {
    connectionCallback_ = callback;
}

void TcpServer::setMessageCallback(MessageCallback callback) {
    messageCallback_ = callback;
}

void TcpServer::setWriteCompleteCallback(WriteCompleteCallback callback) {
    writeCompleteCallback_ = callback;
}

void TcpServer::handleNewConnection(int sockfd, const InetAddress & peerAddr) {
    std::string connName = name_ + "#" + std::to_string(nextConnId_) + " [" + static_cast<std::string>(peerAddr) + "]";
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(threadPool_->getNextLoop(), connName, sockfd, localAddr_, peerAddr);
    ++nextConnId_;

    connections_.insert({std::move(connName), conn});

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::handleRemoveConnection, this, conn));
    conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));

    DLOG(INFO) << "New connection [name = " << conn->name()
               << ", localaddr = " << conn->localAddress()
               << ", peeraddr = " << conn->peerAddress()
               << "]";
}

void TcpServer::handleRemoveConnection(TcpConnectionPtr conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(TcpConnectionPtr conn) {
    loop_->assertInLoopThread();

    connections_.erase(conn->name());
    conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));

    DLOG(INFO) << "Connection removed [name = " << conn->name()
               << ", localaddr = " << conn->localAddress()
               << ", peeraddr = " << conn->peerAddress()
               << "]";
}
