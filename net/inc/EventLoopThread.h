#ifndef __EVENTLOOPTHREAD_H__
#define __EVENTLOOPTHREAD_H__

#include <boost/utility.hpp>
#include <functional>
#include <string>
#include "Thread.h"
#include "Condition.h"

class EventLoop;

class EventLoopThread: public boost::noncopyable {
public:
    using ThreadInitCallback = std::function<void(void)>;
    EventLoopThread(ThreadInitCallback callback = []{}, const std::string & name = std::string());
    ~EventLoopThread();

    EventLoop * startLoop();

private:
    void ThreadFunc();

    ThreadInitCallback initCallback_;
    Thread thread_;

    EventLoop * loop_;
    MutexLock mutex_;
    Condition cond_;
};

#endif //__EVENTLOOPTHREAD_H__
