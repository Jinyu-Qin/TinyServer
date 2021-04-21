#ifndef __POLLER_H__
#define __POLLER_H__

#include <boost/utility.hpp>
#include <vector>
#include <memory>
#include <unordered_map>

class Channel;
class EventLoop;

class Poller: public boost::noncopyable {
public:
    using PollerPtr = std::unique_ptr<Poller>;

    explicit Poller(EventLoop * loop);
    virtual ~Poller();

    virtual void poll(int millisecond, std::vector<Channel *> & activeChannels) = 0;
    virtual void updateChannel(Channel * channel) = 0;
    virtual void removeChannel(Channel * channel) = 0;
    
    bool hasChannel(Channel * channel) const;
    EventLoop * eventLoop() const;

    static PollerPtr createDefaultPoller(EventLoop * loop);

protected:
    std::unordered_map<int, Channel *> channels_;

private:
    EventLoop * loop_;
};

#endif //__POLLER_H__
