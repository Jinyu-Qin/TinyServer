#ifndef __SELECTPOLLER_H__
#define __SELECTPOLLER_H__

#include "Poller.h"
#include <sys/select.h>

class SelectPoller: public Poller {
public:
    explicit SelectPoller(EventLoop * loop);
    ~SelectPoller() override;

    void poll(int milliseconds, std::vector<Channel *> & activeChannels) override;
    void updateChannel(Channel * channel) override;
    void removeChannel(Channel * channel) override;

private:
    fd_set readfds_;
    fd_set writefds_;
    fd_set errorfds_;
    int maxfd_;    // FIXME 这个最大值只增不减
};

#endif //__SELECTPOLLER_H__
