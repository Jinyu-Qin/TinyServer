#include "PollPoller.h"
#include <stdexcept>
#include "Channel.h"

PollPoller::PollPoller(EventLoop * loop)
    : Poller(loop) {
}

PollPoller::~PollPoller() {
}

void PollPoller::poll(int millisecond, std::vector<Channel *> & activeChannels) {
    // FIXME what will happen when the size of pollfds is zero?
    int ret = ::poll(pollfds_.data(), pollfds_.size(), millisecond);
    if(ret == -1) {
        throw std::runtime_error("couldn't poll");
    } else if(ret > 0) {
        int n = pollfds_.size();
        for(int i = 0; i < n; ++i) {
            struct pollfd & pfd = pollfds_[i];
            if(pfd.revents == 0) {
                continue;
            }

            Channel * channel = channels_[pfd.fd];
            channel->setRevents(pfd.revents);
            activeChannels.push_back(channel);
        }
    }
}

void PollPoller::updateChannel(Channel * channel) {
    if(hasChannel(channel)) {
        int index = indices_[channel->fd()];
        struct pollfd & pfd = pollfds_[index];
        pfd.events = channel->events();
    } else {
        int index = pollfds_.size();
        int fd = channel->fd();

        struct pollfd pfd {
            .fd = fd,
            .events = static_cast<short>(channel->events()),
            .revents = 0
        };
        pollfds_.push_back(std::move(pfd));

        indices_[fd] = index;
        channels_[fd] = channel;
    }
}

void PollPoller::removeChannel(Channel * channel) {
    int fd = channel->fd();
    channels_.erase(fd);

    int index = indices_[fd];
    std::swap(pollfds_[index], pollfds_.back());
    indices_[pollfds_[index].fd] = index;
    indices_.erase(fd);
    pollfds_.pop_back();
}
