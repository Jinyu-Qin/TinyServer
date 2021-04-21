#include "Poller.h"
#include <cstdlib>
#include <string>
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
    std::string poller(getenv("TINY_SERVER_POLLER"));
    if(poller == "EPOLL") {
        return PollerPtr(new EpollPoller(loop));
    } else if(poller == "POLL") {
        return PollerPtr(new PollPoller(loop));
    } else if(poller == "SELECT") {
        return PollerPtr(new SelectPoller(loop));
    }
    return PollerPtr(new EpollPoller(loop));
}
