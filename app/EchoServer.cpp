#include "EchoServer.h"
#include <iostream>
#include <string>

EchoServer::EchoServer(EventLoop * loop, const std::string & addr, int port)
    : loop_(loop)
    , tcpServer_(new TcpServer(loop_, addr, port, 0, "EchoServer")) {
}

EchoServer::~EchoServer() {
}

void EchoServer::start(int numThread) {
    tcpServer_->setNewConnectionCallback(std::bind(&EchoServer::onNewConnection, this, std::placeholders::_1));
    tcpServer_->setConnectionClosedCallback(std::bind(&EchoServer::onConnectionClosed, this, std::placeholders::_1));
    tcpServer_->setMessageReceivedCallback(std::bind(&EchoServer::onMessageReceived, this, std::placeholders::_1, std::placeholders::_2));
    tcpServer_->setMessageSentCallback(std::bind(&EchoServer::onMessageSent, this, std::placeholders::_1));
    tcpServer_->start(numThread);
}

void EchoServer::onNewConnection(TcpConnectionPtr conn) {
    std::cout << "new client connected!" << std::endl;
}

void EchoServer::onMessageReceived(TcpConnectionPtr conn, BufferPtr buffer) {
    std::cout << "received (" << buffer->size() << "B): ";
    while (!buffer->empty()) {
        std::cout << std::string(buffer->readBegin(), buffer->continuousSize());
        conn->send(buffer->readBegin(), buffer->continuousSize());
        buffer->readSeek(buffer->continuousSize());
    }
}

void EchoServer::onMessageSent(TcpConnectionPtr conn) {
    // std::cout << "message sent successfully!" << std::endl;
}

void EchoServer::onConnectionClosed(TcpConnectionPtr conn) {
    std::cout << "some client disconnected!" << std::endl;
}
