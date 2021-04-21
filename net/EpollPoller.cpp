#include "EpollPoller.h"
#include <stdexcept>
#include "Channel.h"

EpollPoller::EpollPoller(EventLoop * loop)
    : Poller(loop)
    , epfd_(epoll_create1(EPOLL_CLOEXEC))
    , events_(16) {
    if(epfd_ == -1) {
        throw std::runtime_error("couldn't create epoll");
    }
}

EpollPoller::~EpollPoller() {
    int ret = close(epfd_);
    if(ret == -1) {
        abort();
    }
    
    // 析构函数不应该抛出异常
    // if(ret == -1) {
    //     throw std::runtime_error("couldn't close epoll");
    // }
}

void EpollPoller::poll(int millisecond, std::vector<Channel *> & activeChannels) {
    int ret = epoll_wait(epfd_, events_.data(), events_.size(), millisecond);
    if(ret == -1) {
        throw std::runtime_error("couldn't wait epoll");
    } else if(ret > 0) {
        for(int i = 0; i < ret; ++i) {
            struct epoll_event & event = events_[i];
            Channel * channel = static_cast<Channel *>(event.data.ptr);
            channel->setRevents(event.events);
            activeChannels.push_back(channel);
        }

        // 满了就自动扩容为原来的两倍大小
        if(ret == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    }
}

void EpollPoller::updateChannel(Channel * channel) {
    int fd = channel->fd();
    struct epoll_event event = {};
    event.events = channel->events();
    event.data.ptr = static_cast<void *>(channel);

    int ret;
    if(hasChannel(channel)) {
        ret = epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &event);
    } else {
        ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &event);
        channels_[fd] = channel;
    }
    if(ret == -1) {
        throw std::runtime_error("couldn't update channel to epoll");
    }
}

void EpollPoller::removeChannel(Channel * channel) {
    int fd = channel->fd();
    int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
    channels_.erase(fd);
    if(ret == -1) {
        throw std::runtime_error("couldn't remove channel from epoll");
    }
}
