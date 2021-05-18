#include "TcpConnection.h"
#include "EventLoop.h"
#include "TimeStamp.h"
#include "Channel.h"
#include "Socket.h"
#include <glog/logging.h>
#include <cassert>

TcpConnection::TcpConnection(EventLoop * loop, const std::string & name, int sockfd, const InetAddress & localAddr, const InetAddress & peerAddr)
    : loop_(loop)
    , name_(name)
    , localAddr_(localAddr)
    , peerAddr_(peerAddr)
    , state_(kConnecting)
    , reading_(false)
    , socket_(new Socket(sockfd))
    , channel_(new Channel(loop_, sockfd)) {
    
    // 只有在将this暴露给外部对象的时候，才可以使用shared_from_this()，否则，对象将永生不灭
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    DLOG(INFO) << "TcpConnection::TcpConnection() called, name = " << name_
               << ", sockfd = " << socket_->fd()
               << ", localaddr = " << localAddr_
               << ", peeraddr = " << peerAddr_;
}

TcpConnection::~TcpConnection() {
    DLOG(INFO) << "TcpConnection::~TcpConnection() called, name = " << name_
               << ", sockfd = " << socket_->fd()
               << ", localaddr = " << localAddr_
               << ", peeraddr = " << peerAddr_;
    assert(state_ == kDisconnected);
}

EventLoop * TcpConnection::getLoop() const {
    return loop_;
}

const std::string & TcpConnection::name() const {
    return name_;
}

const InetAddress & TcpConnection::localAddress() const {
    return localAddr_;
}

const InetAddress & TcpConnection::peerAddress() const {
    return peerAddr_;
}

bool TcpConnection::connected() const {
    return state_ == kConnected;
}

bool TcpConnection::disconnected() const {
    return state_ == kDisconnected;
}

void TcpConnection::send(const void * message, size_t size) {
    if(state_ != kConnected) {
        LOG(WARNING) << "Ignore TcpConnection::send(), state = " << stateString(state_);
        return ;
    }

    if(loop_->isInLoopThread()) {
        sendInLoop(message, size);
    } else {
        std::shared_ptr<Buffer> buf = std::make_shared<Buffer>(size);
        buf->write(static_cast<const char *>(message), size);
        void (TcpConnection::*fp)(std::shared_ptr<Buffer>) = &TcpConnection::sendInLoop;
        // FIXME this or shared_from_this()?
        loop_->queueInLoop(std::bind(fp, shared_from_this(), buf));
    }
}

void TcpConnection::send(BufferPtr message) {
    // FIXME what happened if message point to outputBuffer_?
    Buffer::size_type readableSize = message->readableSize();
    send(message->readBegin(), readableSize);
    message->hasRead(readableSize);
}

void TcpConnection::shutdown() {
    if(state_ != kConnected) {
        LOG(WARNING) << "Ignore TcpConnection::shutdown(), state = " << stateString(state_);
        return ;
    }
    state_ = kDisconnecting;
    // FIXME this or shared_from_this()?
    loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, shared_from_this()));
}

void TcpConnection::forceClose() {
    if(state_ != kConnected) {
        LOG(WARNING) << "Ignore TcpConnection::forceClose(), state = " << stateString(state_);
        return ;
    }
    state_ = kDisconnecting;
    // FIXME this or shared_from_this()?
    loop_->runInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
}

void TcpConnection::startRead() {
    // FIXME check state?
    // FIXME this or shared_from_this()?
    loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, shared_from_this()));
}

void TcpConnection::stopRead() {
    // FIXME check state?
    // FIXME this or shared_from_this()?
    loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, shared_from_this()));
}

bool TcpConnection::isReading() const {
    return reading_;
}

void TcpConnection::setConnectionCallback(ConnectionCallback callback) {
    connectionCallback_ = callback;
}

void TcpConnection::setMessageCallback(MessageCallback callback) {
    messageCallback_ = callback;
}

void TcpConnection::setWriteCompleteCallback(WriteCompleteCallback callback) {
    writeCompleteCallback_ = callback;
}

void TcpConnection::setCloseCallback(CloseCallback callback) {
    closeCallback_ = callback;
}

void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    // FIXME 这里脱离了reading_的掌控
    channel_->enableReading();
    state_ = kConnected;

    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    if(state_ == kConnected) {
        state_ = kDisconnected;
        channel_->disableAll();
        
        if(connectionCallback_) {
            connectionCallback_(shared_from_this());
        }
    }
    channel_->remove();
}

void TcpConnection::handleRead(TimeStamp receiveTime) {
    loop_->assertInLoopThread();

    Buffer::size_type nBytes = inputBuffer_.writeFromFd(socket_->fd());
    if(nBytes > 0) {
        // 读到nBytes字节数据
        if(messageCallback_) {
            messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
        }
    } else if(nBytes == 0) {
        // 对端关闭连接
        DLOG(INFO) << "The peer (" << peerAddr_ << ") closed the tcp connection";
        handleClose();
    } else {
        // 出错
        handleError();
    }
}

void TcpConnection::handleWrite() {
    loop_->assertInLoopThread();
    assert(channel_->isWriting());

    Buffer::size_type nBytes = outputBuffer_.readIntoFd(socket_->fd());
    if(nBytes >= 0) {
        // 写入nBytes字节
        if(outputBuffer_.readableSize() == 0) {
            // 缓冲区全部输出
            channel_->disableWriting();
            if(writeCompleteCallback_) {
                writeCompleteCallback_(shared_from_this());
            }

            // 缓冲区数据全部输出，可以关闭连接了
            if(state_ == kDisconnecting) {
                shutdownInLoop();
            }
        }
    } else {
        // 出错
        handleClose();
    }
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    // kDisconnecting状态表示主动关闭，kConnected状态表示被动关闭
    assert(state_ == kDisconnecting || state_ == kConnected);

    state_ = kDisconnected;
    channel_->disableAll();
    if(connectionCallback_) {
        connectionCallback_(shared_from_this());
    }
    if(closeCallback_) {
        closeCallback_(shared_from_this());
    }
}

void TcpConnection::handleError() {
    loop_->assertInLoopThread();

    LOG(FATAL) << "An Error happened in TcpConnection [name = " << name_
               << ", sockfd = " << socket_->fd()
               << ", localaddr = " << localAddr_
               << ", peeraddr = " << peerAddr_ << "]";
}

void TcpConnection::sendInLoop(const void * message, size_t size) {
    loop_->assertInLoopThread();
    assert(state_ == kConnected);

    ssize_t remaining = size;
    ssize_t nBytes = 0;
    bool error = false;
    // 首先尝试直接发送
    if(!channel_->isWriting() && outputBuffer_.readableSize() == 0) {
        nBytes = ::send(socket_->fd(), message, size, 0);
        if(nBytes == remaining) {
            // 全部直接发送完成
            remaining -= nBytes;
            if(writeCompleteCallback_) {
                writeCompleteCallback_(shared_from_this());
            }
        } else if(nBytes >= 0) {
            // 部分直接发送完成
            remaining -= nBytes;
        } else {
            error = true;
            LOG(ERROR) << "Something wrong when call bind() in TcpConnection::sendInLoop(const void * message, size_t size), the errno is " << errno << "(" << strerror(errno) << ")";
            handleError();
        }
    }

    // 然后尝试缓冲发送
    if(!error && remaining > 0) {
        outputBuffer_.write(static_cast<const char *>(message) + nBytes, remaining);
        if(!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}


void TcpConnection::sendInLoop(std::shared_ptr<Buffer> message) {
    loop_->assertInLoopThread();
    sendInLoop(message->readBegin(), message->readableSize());
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    assert(state_ == kDisconnecting);
    
    // 没数据需要发送了才会执行
    if(!channel_->isWriting()) {
        socket_->shutdownWrite();
    }
    // 若还有数据需要发送，则待数据发送完毕后，会自动再次调用shutdownInLoop()
}

void TcpConnection::forceCloseInLoop() {
    loop_->assertInLoopThread();
    assert(state_ == kDisconnecting || state_ == kConnected);

    handleClose();
}

void TcpConnection::startReadInLoop() {
    loop_->assertInLoopThread();
    if(!reading_ || !channel_->isReading()) {
        channel_->enableReading();
        reading_ = true;
    }
}

void TcpConnection::stopReadInLoop() {
    loop_->assertInLoopThread();

    if(reading_ || channel_->isReading()) {
        reading_ = false;
        channel_->disableReading();
    }
}

const std::string & TcpConnection::stateString(TcpConnectionState state) {
    return stateStr[state];
}

const std::string TcpConnection::stateStr[] = {
    "kConnecting",
    "kConnected",
    "kDisconnecting",
    "kDisconnected"
};
