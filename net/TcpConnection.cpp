#include "TcpConnection.h"
#include <sys/socket.h>
#include <stdexcept>
#include "Channel.h"

TcpConnection::TcpConnection(EventLoop * loop, int sockfd, const std::string & name)
    : loop_(loop)
    , sockfd_(sockfd)
    , name_(name)
    , disconnected_(false)
    , channel_(new Channel(loop_, sockfd_))
    , messageReceivedCallback_([](TcpConnectionPtr, BufferPtr){})
    , messageSentCallback_([](TcpConnectionPtr){})
    , closeCallback_([](TcpConnectionPtr){})
    , errorCallback_([](TcpConnectionPtr){})
    , cleanUpCallback_([](TcpConnectionPtr){})
    , bufferIn_(kBufferInCapacity_)
    , bufferOut_(kBufferOutCapacity_) {
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection() {
    if(!disconnected()) {
        shutdown();
    }
}

void TcpConnection::setMessageReceivedCallback(MessageReceivedCallback callback) {
    messageReceivedCallback_ = callback ? callback : [](TcpConnectionPtr, BufferPtr){};
}

void TcpConnection::setMessageSentCallback(MessageSentCallback callback) {
    messageSentCallback_ = callback ? callback : [](TcpConnectionPtr){};
}

void TcpConnection::setCloseCallback(CloseCallback callback) {
    closeCallback_ = callback ? callback : [](TcpConnectionPtr){};
}

void TcpConnection::setErrorCallback(ErrorCallback callback) {
    errorCallback_ = callback ? callback : [](TcpConnectionPtr){};
}

void TcpConnection::setCleanUpCallback(CleanUpCallback callback) {
    cleanUpCallback_ = callback ? callback : [](TcpConnectionPtr){};
}

bool TcpConnection::disconnected() const {
    return disconnected_;
}

const std::string & TcpConnection::name() const {
    return name_;
}

void TcpConnection::send(const char * buf, int len) {
    if(loop_->isInLoopThread()) {
        sendInLoop(buf, len);
    } else {
        std::shared_ptr<char> b(new char[len], std::default_delete<char[]>());
        std::copy(buf, buf + len, b.get());
        void (TcpConnection::*fp)(std::shared_ptr<char>, int) = &TcpConnection::sendInLoop;
        loop_->queueInLoop(std::bind(fp, this, b, len));
    }
}

void TcpConnection::send(const std::string & str) {
    send(str.data(), str.size());
}

void TcpConnection::shutdown() {
    if(disconnected_) {
        return ;
    }
    loop_->queueInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
}

void TcpConnection::start() {
    if(disconnected_) {
        return ;
    }
    loop_->queueInLoop(std::bind(&TcpConnection::startInLoop, this));
}

void TcpConnection::stop() {
    if(disconnected_) {
        return ;
    }
    loop_->queueInLoop(std::bind(&TcpConnection::stopInLoop, this));
}

void TcpConnection::handleRead() {
    if(bufferIn_.full()) {
        handleError();
        return ;
    }

    int len = bufferIn_.continuousAvailable();
    int nBytes = ::recv(channel_->fd(), bufferIn_.writeBegin(), len, 0);
    if(nBytes == -1) {
        handleError();
        return ;
    } else if(nBytes == 0) {
        handleClose();
        return ;
    }
    bufferIn_.writeSeek(nBytes);
    // loop_->queueInLoop(std::bind(messageReceivedCallback_, shared_from_this(), &bufferIn_));
    messageReceivedCallback_(shared_from_this(), &bufferIn_);
}

void TcpConnection::handleWrite() {
    if(!channel_->isWriting() || bufferOut_.empty()) {
        handleError();
        return ;
    }

    int len = bufferOut_.continuousSize();
    int nBytes = ::send(channel_->fd(), bufferOut_.readBegin(), len, 0);
    if(nBytes == -1) {
        handleError();
        return ;
    }
    bufferOut_.readSeek(nBytes);

    if(bufferOut_.empty()) {
        channel_->enableWriting(false);
        loop_->queueInLoop(std::bind(messageSentCallback_, shared_from_this()));
    }
}

void TcpConnection::handleClose() {
    disconnected_ = true;
    if(channel_->isReading()) {
        channel_->enableReading(false);
    }
    if(channel_->isWriting()) {
        channel_->enableWriting(false);
    }
    closeCallback_(shared_from_this());
    cleanUpCallback_(shared_from_this());
    channel_->remove();
}

void TcpConnection::handleError() {
    errorCallback_(shared_from_this());
    cleanUpCallback_(shared_from_this());
    channel_->remove();
}

void TcpConnection::sendInLoop(const char * buf, int len) {
    if(disconnected_) {
        // 连接已关闭，无法发送
        handleError();
        return ;
    }

    int remaining = len;
    if(!channel_->isWriting() && bufferOut_.empty()) {
        // 不缓冲，直接发送
        int nBytes = ::send(channel_->fd(), buf, len, 0);
        if(nBytes == -1) {
            // 发送出错
            handleError();
            return ;
            // throw std::runtime_error("couldn't send some bytes in the buffer");
        }
        remaining -= nBytes;
        if(remaining == 0) {
            // 全部发送完毕，触发回调函数
            loop_->queueInLoop(std::bind(messageSentCallback_, shared_from_this()));
        }
    }
    
    if(remaining > 0){
        // 剩余的字节加入缓冲队列，等待poller触发发送
        if(bufferOut_.available() < remaining) {
            // 缓冲队列容量不足
            handleError();
            return ;
            // throw std::runtime_error("couldn't buffer some bytes to send");
        }
        int nBytes = bufferOut_.write(buf + len - remaining, remaining);
        if(!channel_->isWriting()) {
            channel_->enableWriting(true);
        }
    }
}

void TcpConnection::sendInLoop(std::shared_ptr<char> buf, int len) {
    sendInLoop(buf.get(), len);
}

void TcpConnection::shutdownInLoop() {
    disconnected_ = true;
    if(channel_->isWriting()) {
        channel_->enableWriting(false);
    }
    channel_->enableReading(false);
    int ret = ::close(channel_->fd());
    if(ret == -1) {
        handleError();
    } else {
        handleClose();
    }
}

void TcpConnection::startInLoop() {
    if(!channel_->isReading()) {
        channel_->enableReading(true);
    }
}

void TcpConnection::stopInLoop() {
    if(channel_->isReading()) {
        channel_->enableReading(false);
    }
}

TcpConnection::TcpConnectionPtr TcpConnection::createTcpConnection(EventLoop * loop, int sockfd, const std::string & name) {
    // FIXME 使用make_shared
    return std::shared_ptr<TcpConnection>(new TcpConnection(loop, sockfd, name));
}
