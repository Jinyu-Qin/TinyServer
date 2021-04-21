#ifndef __POLLPOLLER_H__
#define __POLLPOLLER_H__

#include "Poller.h"
#include <poll.h>

class PollPoller: public Poller {
public:
    explicit PollPoller(EventLoop * loop);
    ~PollPoller() override;

    void poll(int millisecond, std::vector<Channel *> & activeChannels) override;
    void updateChannel(Channel * channel) override;
    void removeChannel(Channel * channel) override;

private:
    std::vector<struct pollfd> pollfds_;
    std::unordered_map<int, int> indices_;
};

#endif //__POLLPOLLER_H__
