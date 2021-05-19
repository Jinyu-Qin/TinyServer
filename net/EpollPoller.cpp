#include "EpollPoller.h"
#include "TimeStamp.h"
#include "Channel.h"
#include <sys/epoll.h>
#include <errno.h>
#include <cassert>
#include <glog/logging.h>

EpollPoller::EpollPoller(EventLoop * loop)
    : Poller(loop)
    , epfd_(::epoll_create1(EPOLL_CLOEXEC))
    , events_(kInitEventListSize) {
    if(epfd_ < 0) {
        LOG(FATAL) << "Can't create epoll";
    }
}

EpollPoller::~EpollPoller() {
    ::close(epfd_);
}

TimeStamp EpollPoller::poll(int timeoutMs, ChannelList & activeChannels) {
    assertInLoopThread();
    DLOG(INFO) << "Size of channels_ = " << channels_.size();
    int numEvents = ::epoll_wait(epfd_, events_.data(), events_.size(), timeoutMs);
    TimeStamp now = TimeStamp::now();

    if(numEvents > 0) {
        DLOG(INFO) << "Epoll caught " << numEvents << " events";

        // 填充activeChannels
        activeChannels.clear();
        for(int i = 0; i < numEvents; ++i) {
            ChannelPtr channel = static_cast<ChannelPtr>(events_[i].data.ptr);
            assert(hasChannel(channel));
            channel->setRevents(events_[i].events);
            activeChannels.push_back(channel);
        }

        // 如果events满了，则进行扩容
        if(numEvents == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if(numEvents == 0) {
        DLOG(INFO) << "Epoll timeout after " << timeoutMs << " ms";
    } else {
        if(errno != EINTR) {
            LOG(FATAL) << "Something wrong when call epoll_wait(), the errno is " << errno << "(" << strerror(errno) << ")";
        }
    }

    return now;
}

void EpollPoller::updateChannel(ChannelPtr channel) {
    assertInLoopThread();

    if(hasChannel(channel)) {
        // 更新channel
        struct epoll_event event;
        memset(&event, 0, sizeof(event));
        event.data.ptr = static_cast<void *>(channel);
        event.events = channel->events();

        // FIXME channel中如果没有事件了，应将其从poller中移除
        
        if(::epoll_ctl(epfd_, EPOLL_CTL_MOD, channel->fd(), &event) == -1) {
            LOG(FATAL) << "Something wrong when call epoll_ctl(), the errno is " << errno << "(" << strerror(errno) << ")";
        } else {
            DLOG(INFO) << "Modified epoll event, fd = " << channel->fd() << ", events = " << channel->events();
        }
    } else {
        // 添加channel
        struct epoll_event event;
        memset(&event, 0, sizeof(event));
        event.data.ptr = static_cast<void *>(channel);
        event.events = channel->events();
        
        if(::epoll_ctl(epfd_, EPOLL_CTL_ADD, channel->fd(), &event) == -1) {
            LOG(FATAL) << "Something wrong when call epoll_ctl(), the errno is " << errno << "(" << strerror(errno) << ")";
        } else {
            channels_.insert({channel->fd(), channel});
            DLOG(INFO) << "Added epoll event, fd = " << channel->fd() << ", events = " << channel->events();
        }
    }
}

void EpollPoller::removeChannel(ChannelPtr channel) {
    assertInLoopThread();
    assert(hasChannel(channel));

    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.data.ptr = channel;
    event.events = channel->events();

    if(::epoll_ctl(epfd_, EPOLL_CTL_DEL, channel->fd(), &event) == -1) {
        LOG(FATAL) << "Something wrong when call epoll_ctl(), the errno is " << errno << "(" << strerror(errno) << ")";
    } else {
        channels_.erase(channel->fd());
        DLOG(INFO) << "Deleted epoll event, fd = " << channel->fd() << ", events = " << channel->events();
    }
}
