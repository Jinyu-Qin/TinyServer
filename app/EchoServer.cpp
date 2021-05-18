#include "EchoServer.h"
#include "TcpServer.h"
#include "EventLoop.h"
#include "TimeStamp.h"
#include "TcpConnection.h"
#include <cassert>
#include <cstdio>

EchoServer::EchoServer(EventLoop * loop, const std::string & name, const InetAddress & localAddr)
    : loop_(loop)
    , name_(name)
    , localAddr_(localAddr)
    , started_(false) {
}

EchoServer::~EchoServer() {
}

EventLoop * EchoServer::getLoop() const {
    return loop_;
}

const std::string & EchoServer::name() const {
    return name_;
}

const InetAddress & EchoServer::localAddress() const {
    return localAddr_;
}

void EchoServer::start(int NumThread) {
    loop_->assertInLoopThread();
    assert(!started_);
    started_ = true;

    tcpServer_.reset(new TcpServer(loop_, localAddr_, name_));
    tcpServer_->setThreadNum(NumThread);
    tcpServer_->setConnectionCallback(std::bind(&EchoServer::handleConnection, this, std::placeholders::_1));
    tcpServer_->setMessageCallback(std::bind(&EchoServer::handleMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    tcpServer_->start();
}

void EchoServer::stop() {
    assert(started_);
    started_ = false;

    tcpServer_.release();
}

void EchoServer::handleConnection(TcpConnectionPtr conn) {
    if(conn->connected()) {
        printf("New connection (%s)\n", conn->name().c_str());

        std::string msg = "Welcome to " + name_ + "!\n";
        conn->send(msg.c_str(), msg.size());
    } else if(conn->disconnected()) {
        printf("Remove connection (%s)\n", conn->name().c_str());
    }
}

void EchoServer::handleMessage(TcpConnectionPtr conn, BufferPtr message, TimeStamp receiveTime) {
    conn->send(message);
}
