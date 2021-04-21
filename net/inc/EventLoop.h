#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__

#include <boost/utility.hpp>
#include <functional>
#include "Poller.h"
#include "Mutex.h"

class EventLoop: public boost::noncopyable {
    using PollerPtr = std::unique_ptr<Poller>;
    using ChannelPtr = std::unique_ptr<Channel>;
public:
    using Task = std::function<void(void)>;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    void updateChannel(Channel * channel);
    void removeChannel(Channel * channel);
    bool hasChannel(Channel * channel);

    bool taskHandling() const;
    bool eventHandling() const;

    bool isInLoopThread() const;
    pid_t tid() const;
    void runInLoop(Task task);
    void queueInLoop(Task task);
    int queueSize();
    void wakeup();

    void * operator new(std::size_t size) = delete;
    void operator delete(void * obj) = delete;

private:
    int createEventFd();
    void handleWakeupRead();

    bool looping_;
    bool taskHandling_;
    bool eventHandling_;
    pid_t tid_;

    PollerPtr poller_;
    ChannelPtr wakeupChannel_;

    std::vector<Task> tasks_;
    MutexLock mutex_;
};

#endif //__EVENTLOOP_H__
