#ifndef __EVENTLOOPTHREAD_H__
#define __EVENTLOOPTHREAD_H__

#include <boost/utility.hpp>
#include <functional>
#include <string>
#include "Mutex.h"
#include "Condition.h"
#include "Thread.h"

class EventLoop;

class EventLoopThread: public boost::noncopyable {
public:
    using ThreadInitCallback    = std::function<void(EventLoop *)>;

    EventLoopThread(const ThreadInitCallback & callback = ThreadInitCallback(), const std::string & name = std::string());
    ~EventLoopThread();

    // 开始一个IO线程，并返回EventLoop对象
    EventLoop * startLoop();

private:
    // 线程函数
    void threadFunc();

    EventLoop * loop_;
    ThreadInitCallback initCallback_;
    Thread thread_;

    MutexLock mutex_;
    Condition cond_;
};

#endif //__EVENTLOOPTHREAD_H__
