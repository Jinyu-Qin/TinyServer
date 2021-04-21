#include "Channel.h"
#include <poll.h>
#include "EventLoop.h"

Channel::Channel(EventLoop * loop, int fd)
    : fd_(fd)
    , events_(0)
    , revents_(0)
    , readCallback_([]{})
    , writeCallback_([]{})
    , closeCallback_([]{})
    , errorCallback_([]{})
    , loop_(loop) {
}

Channel::~Channel() {
}

int Channel::fd() const {
    return fd_;
}

int Channel::events() const {
    return events_;
}

void Channel::setRevents(int revents) {
    revents_ = revents;
}

void Channel::setReadCallback(Callback callback) {
    readCallback_ = callback;
}

void Channel::setWriteCallback(Callback callback) {
    writeCallback_ = callback;
}

void Channel::setCloseCallback(Callback callback) {
    closeCallback_ = callback;
}

void Channel::setErrorCallback(Callback callback) {
    errorCallback_ = callback;
}

void Channel::enableReading(bool enable) {
    events_ = enable ? (events_ | POLLIN) : (events_ & ~POLLIN);
    update();
}

void Channel::enableWriting(bool enable) {
    events_ = enable ? (events_ | POLLOUT) : (events_ & ~POLLOUT);
    update();
}

bool Channel::isReading() const {
    return events_ & POLLIN;
}

bool Channel::isWriting() const {
    return events_ & POLLOUT;
}

void Channel::handleEvent() {
    if(revents_ & (POLLRDHUP | POLLHUP)) {
        errorCallback_();
    }
    if(revents_ & POLLIN) {
        readCallback_();   // 正常关闭也会触发
    }
    if(revents_ & POLLOUT) {
        writeCallback_();
    }
    // POLLNVAL：文件描述符没有打开
    if(revents_ & (POLLNVAL | POLLERR)) {
        errorCallback_();
    }
}

void Channel::remove() {
    loop_->removeChannel(this);
}

void Channel::update() {
    loop_->updateChannel(this);
}
