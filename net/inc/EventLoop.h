#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__

#include <boost/utility.hpp>
#include <functional>
#include <vector>
#include <memory>
#include "Mutex.h"

class Channel;
class Poller;

class EventLoop: public boost::noncopyable {
public:
    using Functor       = std::function<void(void)>;
    using ChannelPtr    = Channel *;
    using ChannelList   = std::vector<ChannelPtr>;
    using PollerPtr     = std::unique_ptr<Poller>;

    EventLoop();
    ~EventLoop();

    // 事件循环
    void loop();
    // 退出事件循环
    void quit();
    // 如果在IO线程中调用，则同步执行task，否则，将task加入到任务队列
    void runInLoop(Functor task);
    // 将task加入到任务队列
    void queueInLoop(Functor task);

    // FIXME 加入定时器任务

    // 唤醒Poller::poll()
    void wakeup();
    // 添加或更新channel
    void updateChannel(ChannelPtr channel);
    // 移除channel
    void removeChannel(ChannelPtr channel);
    // 判断channel是否在当前EventLoop中
    bool hasChannel(ChannelPtr channel) const;

    // 断言当前是否处于I/O线程
    void assertInLoopThread() const;
    // 判断当前是否处于I/O线程
    bool isInLoopThread() const;

private:
    // wakeup()唤醒后的回调函数
    void handleWakeUp();
    // 执行任务队列中的task
    void doPendingFunctors();

    int wakeupFd_;                  // 用于唤醒Poller::poll()的eventfd
    std::unique_ptr<Channel> wakeupChannel_;    // 封装了用于唤醒的wakeupFd_及其事件处理函数

    bool quit_;                     // 用于指示退出loop()
    bool looping_;                  // 用于标识loop()的状态
    bool eventHandling_;            // 用于标识事件的处理状态
    bool callingPendingFunctors_;   // 用于标识task的处理状态

    const pid_t threadId_;          // EventLoop所在的线程

    PollerPtr poller_;              // Poller

    mutable MutexLock mutex_;       // 用于保护任务队列
    std::vector<Functor> pendingFunctors_;  // 任务队列

    static constexpr int kPollTimeMs = 10000;   // poll超时时间
};

#endif //__EVENTLOOP_H__
