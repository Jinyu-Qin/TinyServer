#ifndef __EVENTLOOPTHREADPOOL_H__
#define __EVENTLOOPTHREADPOOL_H__

#include <boost/utility.hpp>
#include <vector>
#include <memory>
#include "EventLoopThread.h"

class EventLoopThreadPool: public boost::noncopyable {
    using EventLoopThreadPtr = std::unique_ptr<EventLoopThread>;
public:
    using ThreadInitCallback = typename EventLoopThread::ThreadInitCallback;

    EventLoopThreadPool(EventLoop * loop, const std::string & name = std::string());
    ~EventLoopThreadPool();

    void setThreadInitCallback(ThreadInitCallback callback);

    void start(int numThread);
    void stop();

    const std::string & name() const;

    EventLoop * nextLoop();
    std::vector<EventLoop *> allLoops() const;

public:
    EventLoop * loop_;
    std::string name_;
    ThreadInitCallback initCallback_;

    std::vector<EventLoopThreadPtr> threads_;
    std::vector<EventLoop *> loops_;
    int index_;
};

#endif //__EVENTLOOPTHREADPOOL_H__
