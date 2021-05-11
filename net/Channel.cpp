#include "Channel.h"
#include "TimeStamp.h"
#include "EventLoop.h"
#include <poll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop * loop, int fd)
    : loop_(loop)
    , fd_(fd)
    , events_(0)
    , revents_(0) {
}

Channel::~Channel() {
}

EventLoop * Channel::ownerLoop() const {
    return loop_;
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

void Channel::setReadCallback(ReadEventCallback callback) {
    readCallback = callback;
}

void Channel::setWriteCallback(EventCallback callback) {
    writeCallback = callback;
}

void Channel::setCloseCallback(EventCallback callback) {
    closeCallback = callback;
}

void Channel::setErrorCallback(EventCallback callback) {
    errorCallback = callback;
}

void Channel::handleEvent(TimeStamp time) {
    // 处理可读事件
    if(revents_ & kReadEvent) {
        if(readCallback) {
            readCallback(time);
        }
    }
    // 处理可写事件
    if(revents_ & kWriteEvent) {
        if(writeCallback) {
            writeCallback();
        }
    }

    // FIXME 处理关闭、错误事件
}

void Channel::enableReading() {
    events_ |= kReadEvent;
    update();
}

void Channel::enableWriting() {
    events_ |= kWriteEvent;
    update();
}

void Channel::disableReading() {
    events_ &= ~kReadEvent;
    update();
}

void Channel::disableWriting() {
    events_ &= ~kWriteEvent;
    update();
}

void Channel::disableAll() {
    events_ &= ~(kReadEvent | kWriteEvent);
    update();
}

bool Channel::isReading() const {
    return static_cast<bool>(events_ & kReadEvent);
}

bool Channel::isWriting() const {
    return static_cast<bool>(events_ & kWriteEvent);
}

void Channel::remove() {
    loop_->removeChannel(this);
}

void Channel::update() {
    loop_->updateChannel(this);
}
