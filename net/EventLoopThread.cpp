#include "EventLoopThread.h"
#include <cassert>
#include <glog/logging.h>
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback & callback, const std::string & name)
    : loop_(nullptr)
    , initCallback_(callback)
    , thread_(std::bind(&EventLoopThread::threadFunc, this), name)
    , mutex_()
    , cond_(mutex_) {
}

EventLoopThread::~EventLoopThread() {
    if(loop_ != nullptr) {
        // FIXME 有可能执行析构的时候，loop_还正在创建中
        loop_->quit();
        thread_.join();
    }
}

EventLoop * EventLoopThread::startLoop() {
    assert(!thread_.started());
    thread_.start();

    EventLoop * loop = nullptr;

    {
        MutexLockGuard lock(mutex_);
        while(loop_ == nullptr) {
            cond_.wait();
        }
        loop = loop_;
    }

    return loop;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    
    if(initCallback_) {
        initCallback_(&loop);
    }

    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }

    loop.loop();

    MutexLockGuard lock(mutex_);
    loop_ = nullptr;
}
