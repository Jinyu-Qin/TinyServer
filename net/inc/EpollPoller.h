#ifndef __EPOLLPOLLER_H__
#define __EPOLLPOLLER_H__

#include "Poller.h"
#include <sys/epoll.h>

class EpollPoller: public Poller {
public:
    explicit EpollPoller(EventLoop * loop);
    ~EpollPoller() override;

    void poll(int millisecond, std::vector<Channel *> & activeChannels) override;
    void updateChannel(Channel * channel) override;
    void removeChannel(Channel * channel) override;

private:
    int epfd_;
    std::vector<struct epoll_event> events_;
};

#endif //__EPOLLPOLLER_H__
