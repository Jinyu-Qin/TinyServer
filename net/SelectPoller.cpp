#include "SelectPoller.h"
#include <poll.h>
#include <stdexcept>
#include "Channel.h"

SelectPoller::SelectPoller(EventLoop * loop)
    : Poller(loop)
    , maxfd_(0) {
    FD_ZERO(&readfds_);
    FD_ZERO(&writefds_);
    FD_ZERO(&errorfds_);
}

SelectPoller::~SelectPoller() {
}

void SelectPoller::poll(int milliseconds, std::vector<Channel *> & activeChannels) {
    struct timeval tv {
        .tv_sec = milliseconds / 1000,
        .tv_usec = milliseconds % 1000 * 1000
    };
    fd_set readfds = readfds_;
    fd_set writefds = writefds_;
    fd_set errorfds = errorfds_;
    int ret = select(maxfd_ + 1, &readfds, &writefds, &errorfds, &tv);
    if(ret == -1) {
        throw std::runtime_error("couldn't select");
    } else if(ret > 0) {
        for(int fd = 0; fd <= maxfd_; ++fd) {
            int revents = 0;
            
            revents |= FD_ISSET(fd, &readfds) ? POLLIN : 0;
            revents |= FD_ISSET(fd, &writefds) ? POLLOUT : 0;
            revents |= FD_ISSET(fd, &errorfds) ? POLLERR : 0;

            if(revents == 0) {
                continue;
            }

            Channel * channel = channels_[fd];
            channel->setRevents(revents);
            activeChannels.push_back(channel);
        }
    }
}

void SelectPoller::updateChannel(Channel * channel) {
    int fd = channel->fd();
    int events = channel->events();
    if(hasChannel(channel)) {
        if(events & POLLIN) {
            FD_SET(fd, &readfds_);
        } else {
            FD_CLR(fd, &readfds_);
        }

        if(events & POLLOUT) {
            FD_SET(fd, &writefds_);
        } else {
            FD_CLR(fd, &writefds_);
        }

        if(events & POLLERR) {
            FD_SET(fd, &errorfds_);
        } else {
            FD_CLR(fd, &errorfds_);
        }
    } else {
        if(events & POLLIN) {
            FD_SET(fd, &readfds_);
        }

        if(events & POLLOUT) {
            FD_SET(fd, &writefds_);
        }

        if(events & POLLERR) {
            FD_SET(fd, &errorfds_);
        }

        maxfd_ = std::max(maxfd_, fd);
        channels_[fd] = channel;
    }
}

void SelectPoller::removeChannel(Channel * channel) {
    int fd = channel->fd();
    FD_CLR(fd, &readfds_);
    FD_CLR(fd, &writefds_);
    FD_CLR(fd, &errorfds_);
    channels_.erase(fd);
    // FIXME 移除channel时如何更新maxfd_
}

