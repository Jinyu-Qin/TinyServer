#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include <cassert>

EventLoopThreadPool::EventLoopThreadPool(EventLoop * loop, const std::string & name)
    : loop_(loop)
    , name_(name)
    , started_(false)
    , numThreads_(0)
    , next_(0)
    , threads_()
    , loops_() {
}

EventLoopThreadPool::~EventLoopThreadPool() {
}

void EventLoopThreadPool::setThreadNum(int numThreads) {
    loop_->assertInLoopThread();
    assert(!started_);

    numThreads_ = numThreads;
}

void EventLoopThreadPool::start(const ThreadInitCallback & callback) {
    loop_->assertInLoopThread();
    assert(!started_);

    started_ = true;

    for(int i = 0; i < numThreads_; ++i) {
        threads_.push_back(std::unique_ptr<EventLoopThread>(new EventLoopThread(callback, name_ + "#" + std::to_string(i))));
        loops_.push_back(threads_.back()->startLoop());
    }

    if(numThreads_ == 0 && callback) {
        // 如果线程池为空，则直接使用其所属的线程
        callback(loop_);
    }
}

EventLoop * EventLoopThreadPool::getNextLoop() {
    loop_->assertInLoopThread();
    assert(started_);

    EventLoop * loop = loop_;  // 线程池为空则返回其所属的loop

    // Round Robin
    if(!loops_.empty()) {
        loop = loops_[next_];
        next_ = (next_ + 1) % loops_.size();
    }

    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops() {
    loop_->assertInLoopThread();
    assert(started_);

    if(loops_.empty()) {
        // 线程池为空就返回其所属的loop
        return {loop_};
    } else {
        return loops_;
    }
}

bool EventLoopThreadPool::started() const {
    loop_->assertInLoopThread();
    return started_;
}

const std::string & EventLoopThreadPool::name() const {
    loop_->assertInLoopThread();
    return name_;
}

