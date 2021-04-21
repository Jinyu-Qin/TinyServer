#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop * loop, const std::string & name)
    : loop_(loop)
    , name_(name)
    , initCallback_([]{})
    , index_(0) {
}

EventLoopThreadPool::~EventLoopThreadPool() {
    stop();
}

void EventLoopThreadPool::setThreadInitCallback(ThreadInitCallback callback) {
    initCallback_ = callback ? callback : []{};
}

void EventLoopThreadPool::start(int numThread) {
    if(numThread == 0) {
        initCallback_();
        return ;
    }

    for(int i = 0; i < numThread; ++i) {
        char buf[32];
        snprintf(buf, sizeof(buf), "EventLoop#%d", i);
        EventLoopThreadPtr thread(new EventLoopThread(initCallback_, buf));
        loops_.push_back(thread->startLoop());
        threads_.emplace_back(std::move(thread));
    }
}

void EventLoopThreadPool::stop() {
    loops_.clear();
    index_ = 0;
    threads_.clear();
}

const std::string & EventLoopThreadPool::name() const {
    return name_;
}

EventLoop * EventLoopThreadPool::nextLoop() {
    if(loops_.empty()) {
        return loop_;
    }
    EventLoop * loop;
    loop = loops_[index_];
    index_ = (index_ + 1) % loops_.size();
    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::allLoops() const {
    if(loops_.empty()) {
        return {loop_};
    }
    return loops_;
}
