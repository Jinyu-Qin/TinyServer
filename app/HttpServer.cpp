#include "HttpServer.h"
#include "EventLoop.h"
#include "TimeStamp.h"
#include "TcpServer.h"
#include "TcpConnection.h"
#include "HttpContext.h"
#include "HttpService.h"
#include <cassert>

HttpServer::HttpServer(EventLoop * loop, const std::string & name, const InetAddress & localAddr, const std::string & root)
    : loop_(loop)
    , name_(name)
    , localAddr_(localAddr)
    , root_(root)
    , started_(false)
    , service_(new HttpService(root_)) {
}

HttpServer::~HttpServer() {
}

EventLoop * HttpServer::getLoop() const {
    return loop_;
}

const std::string & HttpServer::name() const {
    return name_;
}

const InetAddress & HttpServer::localAddress() const {
    return localAddr_;
}

void HttpServer::start(int numThreads) {
    loop_->assertInLoopThread();
    assert(!started_);
    assert(numThreads >= 0);

    started_ = true;
    tcpServer_.reset(new TcpServer(loop_, localAddr_, name_));
    tcpServer_->setThreadNum(numThreads);
    tcpServer_->setConnectionCallback(std::bind(&HttpServer::handleConnection, this, std::placeholders::_1));
    tcpServer_->setMessageCallback(std::bind(&HttpServer::handleMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    tcpServer_->start();
}

void HttpServer::stop() {
    loop_->assertInLoopThread();
    assert(started_);

    started_ = false;
    tcpServer_.release();
}

void HttpServer::handleConnection(TcpConnectionPtr conn) {
    if(conn->connected()) {
        // FIXME unsafe
        auto it = contexts_.find(conn->hashCode());
        assert(it == contexts_.cend());

        HttpContextPtr context(std::make_shared<HttpContext>(conn));
        context->setServiceCallback(std::bind(&HttpService::service, service_.get(), std::placeholders::_1, std::placeholders::_2));

        MutexLockGuard lock(mutex_);
        contexts_.insert({conn->hashCode(), context});
    } else if(conn->disconnected()) {
        // FIXME unsafe
        auto it = contexts_.find(conn->hashCode());
        assert(it != contexts_.cend());

        MutexLockGuard lock(mutex_);
        contexts_.erase(conn->hashCode());
    }
}

void HttpServer::handleMessage(TcpConnectionPtr conn, BufferPtr message, TimeStamp receiveTime) {
    // FIXME unsafe
    auto it = contexts_.find(conn->hashCode());
    assert(it != contexts_.cend());

    HttpContextPtr context(it->second);
    context->process(message, receiveTime);
}
