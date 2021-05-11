#ifndef __POLLER_H__
#define __POLLER_H__

#include <boost/utility.hpp>
#include <unordered_map>
#include <vector>
#include <memory>

class Channel;
class EventLoop;
class TimeStamp;

class Poller: public boost::noncopyable {
public:
    using ChannelPtr    = Channel *;               // 只有原始指针才能存放在epoll_event的data.ptr中
    using ChannelList   = std::vector<ChannelPtr>;
    using PollerPtr     = std::unique_ptr<Poller>;

    Poller(EventLoop * loop);
    virtual ~Poller();

    // I/O复用
    virtual TimeStamp poll(int timeoutMs, ChannelList & activeChannels) = 0;
    // 添加或更新Channel
    virtual void updateChannel(ChannelPtr channel) = 0;
    // 移除Channel
    virtual void removeChannel(ChannelPtr channel) = 0;

    // 判断是否包含Channel
    bool hasChannel(ChannelPtr channel);

    // 获取所属EventLoop
    EventLoop * ownerLoop() const;
    // 断言是否处于I/O线程中
    void assertInLoopThread() const;

    static PollerPtr createPoller(EventLoop * loop);

protected:
    using ChannelMap    = std::unordered_map<int, ChannelPtr>; // fd - channel映射
    ChannelMap channels_;

private:
    EventLoop * loop_;
};

#endif //__POLLER_H__
