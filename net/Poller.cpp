#include "Poller.h"
#include "EpollPoller.h"
#include "PollPoller.h"
#include "SelectPoller.h"
#include "Channel.h"
#include "EventLoop.h"

Poller::Poller(EventLoop * loop)
    : loop_(loop) {
}

Poller::~Poller() {
}

bool Poller::hasChannel(Channel * channel) const {
    auto iter = channels_.find(channel->fd());
    return iter != channels_.cend();
}

EventLoop * Poller::eventLoop() const {
    return loop_;
}

Poller::PollerPtr Poller::createDefaultPoller(EventLoop * loop) {
    return PollerPtr(new EpollPoller(loop));
}
