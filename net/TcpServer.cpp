#include "TcpServer.h"
#include <fcntl.h>
#include <stdexcept>

TcpServer::TcpServer(EventLoop * loop, const std::string & addr, int port, int maxConn, const std::string & name)
    : loop_(loop)
    , address_(addr)
    , port_(port)
    , maxConnection_(maxConn)
    , numConnection_(0)
    , name_(name)
    , pool_(new EventLoopThreadPool(loop_, "TcpServerEventLoopThreadPool"))
    , serverSocket_(new ServerSocket())
    , serverSocketChannel_(new Channel(loop_, serverSocket_->fd()))
    , newConnectionCallback_([](TcpConnectionPtr){})
    , messageReceivedCallback_([](TcpConnectionPtr, BufferPtr){})
    , messageSentCallback_([](TcpConnectionPtr){})
    , connectionClosedCallback_([](TcpConnectionPtr){})
    , connectionErrorCallback_([](TcpConnectionPtr){})
    , threadInitCallback_([]{}) {
}

TcpServer::~TcpServer() {
}

void TcpServer::setNewConnectionCallback(NewConnectionCallback callback) {
    newConnectionCallback_ = callback ? callback : [](TcpConnectionPtr){};
}

void TcpServer::setMessageReceivedCallback(MessageReceivedCallback callback) {
    messageReceivedCallback_ = callback ? callback : [](TcpConnectionPtr, BufferPtr){};
}

void TcpServer::setMessageSentCallback(MessageSentCallback callback) {
    messageSentCallback_ = callback ? callback : [](TcpConnectionPtr){};
}

void TcpServer::setConnectionClosedCallback(ConnectionClosedCallback callback) {
    connectionClosedCallback_ = callback ? callback : [](TcpConnectionPtr){};
}

void TcpServer::setConnectionErrorCallback(ConnectionErrorCallback callback) {
    connectionErrorCallback_ = callback ? callback : [](TcpConnectionPtr){};
}

void TcpServer::setThreadInitCallback(ThreadInitCallback callback) {
    threadInitCallback_ = callback ? callback : []{};
}

void TcpServer::start(int numThread) {
    serverSocket_->setNonblocking(true);
    serverSocket_->setReuseAddress(true);
    serverSocket_->bind(address_.c_str(), port_);
    serverSocket_->listen();

    serverSocketChannel_->setReadCallback(std::bind(&TcpServer::handleNewConnection, this));
    serverSocketChannel_->setErrorCallback(std::bind(&TcpServer::handleError, this));
    serverSocketChannel_->enableReading(true);

    pool_->setThreadInitCallback(threadInitCallback_);
    pool_->start(numThread);
}

const std::string & TcpServer::serverAddress() const {
    return address_;
}
;
int TcpServer::serverPort() const {
    return port_;
}

const std::string & TcpServer::name() const {
    return name_;
}

void TcpServer::handleNewConnection() {
    try {
        while(true) {
            int fd = serverSocket_->accept();
            // FIXME numConnection_线程不安全
            ++numConnection_;
            if(maxConnection_ != 0 && numConnection_ > maxConnection_) {
                --numConnection_;
                int ret = ::close(fd);
                if(ret == -1) {
                    handleError();
                }
            }

            setNonblocking(fd);

            EventLoop * loop = pool_->nextLoop();
            TcpConnectionPtr conn = TcpConnection::createTcpConnection(loop, fd, "ClientConnection");
            connections_.emplace_back(conn);

            conn->setMessageReceivedCallback(messageReceivedCallback_);
            conn->setMessageSentCallback(messageSentCallback_);
            conn->setCloseCallback(connectionClosedCallback_);
            conn->setErrorCallback(connectionErrorCallback_);
            conn->setCleanUpCallback(std::bind(&TcpServer::handleRemoveConnection, this, std::placeholders::_1));
            conn->start();

            newConnectionCallback_(conn);
        }
    } catch (...) {
    }
}

void TcpServer::handleRemoveConnection(TcpConnectionPtr conn) {
    --numConnection_;
}

void TcpServer::handleError() {
}

void TcpServer::setNonblocking(int fd) {
    int opt = fcntl(fd, F_GETFL);
    if(opt == -1) {
        throw std::runtime_error("couldn't get flag of fd");
    }
    opt |= O_NONBLOCK;
    int ret = fcntl(fd, F_SETFL, opt);
    if(ret == -1) {
        throw std::runtime_error("couldn't set flag of fd");
    }
}