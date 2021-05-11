#include "Poller.h"
#include "Channel.h"
#include "EpollPoller.h"
#include "EventLoop.h"

Poller::Poller(EventLoop * loop)
    : loop_(loop) {
}

Poller::~Poller() {
}

bool Poller::hasChannel(ChannelPtr channel) {
    assertInLoopThread();
    ChannelMap::const_iterator iter = channels_.find(channel->fd());
    return iter != channels_.cend() && iter->second == channel;
}

EventLoop * Poller::ownerLoop() const {
    return loop_;
}

void Poller::assertInLoopThread() const {
    loop_->assertInLoopThread();
}

Poller::PollerPtr Poller::createPoller(EventLoop * loop) {
    return PollerPtr(new EpollPoller(loop));
}