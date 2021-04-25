#include "EchoServer.h"
#include <iostream>
#include <string>

EchoServer::EchoServer(EventLoop * loop, const std::string & addr, int port)
    : loop_(loop)
    , tcpServer_(new TcpServer(loop_, addr, port, 0, "EchoServer"))
    , logger_(Logger::getLogger()) {
}

EchoServer::~EchoServer() {
}

void EchoServer::start(int numThread) {
    tcpServer_->setNewConnectionCallback(std::bind(&EchoServer::onNewConnection, this, std::placeholders::_1));
    tcpServer_->setConnectionClosedCallback(std::bind(&EchoServer::onConnectionClosed, this, std::placeholders::_1));
    tcpServer_->setMessageReceivedCallback(std::bind(&EchoServer::onMessageReceived, this, std::placeholders::_1, std::placeholders::_2));
    tcpServer_->setMessageSentCallback(std::bind(&EchoServer::onMessageSent, this, std::placeholders::_1));
    tcpServer_->start(numThread);
    logger_->log(Logger::DEBUG, "echo server started!");
}

void EchoServer::onNewConnection(TcpConnectionPtr conn) {
    logger_->log(Logger::DEBUG, "new client connected!");
}

void EchoServer::onMessageReceived(TcpConnectionPtr conn, BufferPtr buffer) {
    logger_->log(Logger::DEBUG, "received %d bytes", buffer->size());
    while (!buffer->empty()) {
        conn->send(buffer->readBegin(), buffer->continuousSize());
        buffer->readSeek(buffer->continuousSize());
    }
}

void EchoServer::onMessageSent(TcpConnectionPtr conn) {
    logger_->log(Logger::DEBUG, "message sent successfully!");
}

void EchoServer::onConnectionClosed(TcpConnectionPtr conn) {
    logger_->log(Logger::DEBUG, "some client disconnected!");
}
