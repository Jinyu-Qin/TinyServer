#ifndef __EPOLLPOLLER_H__
#define __EPOLLPOLLER_H__

#include "Poller.h"

struct epoll_event;

class EpollPoller: public Poller {
public:
    EpollPoller(EventLoop * loop);
    ~EpollPoller() override;

    TimeStamp poll(int timeoutMs, ChannelList & activeChannels) override;
    void updateChannel(ChannelPtr channel) override;
    void removeChannel(ChannelPtr channel) override;

private:
    int epfd_;
    std::vector<struct epoll_event> events_;

    static constexpr int kInitEventListSize = 16;
};

#endif //__EPOLLPOLLER_H__
