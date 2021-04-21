#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(ThreadInitCallback callback, const std::string & name)
    : initCallback_(callback ? callback : []{})
    , thread_(std::bind(&EventLoopThread::ThreadFunc, this))
    , loop_(nullptr)
    , mutex_()
    , cond_(mutex_) {
}

EventLoopThread::~EventLoopThread() {
    if(loop_ != nullptr) {
        loop_->quit();
        thread_.join();  // 应该可以省略
    }
}

EventLoop * EventLoopThread::startLoop() {
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

void EventLoopThread::ThreadFunc() {
    EventLoop loop;

    initCallback_();

    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }

    loop_->loop();
    loop_ = nullptr;
}