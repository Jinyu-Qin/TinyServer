#include "EventLoop.h"
#include "CurrentThread.h"
#include "Poller.h"
#include "Channel.h"
#include "TimeStamp.h"
#include <sys/eventfd.h>
#include <glog/logging.h>
#include <cassert>
#include <errno.h>

__thread EventLoop * loopInThisThread = nullptr;

EventLoop::EventLoop()
    : wakeupFd_(eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK))
    , wakeupChannel_(new Channel(this, wakeupFd_))
    , quit_(false)
    , looping_(false)
    , eventHandling_(false)
    , callingPendingFunctors_(false)
    , threadId_(CurrentThread::tid())
    , poller_(Poller::createPoller(this))
    , mutex_()
    , pendingFunctors_() {
    if(wakeupFd_ == -1) {
        LOG(FATAL) << "Something wrong when call eventfd(), the errno is " << errno << "(" << strerror(errno) << ")";
    }

    DLOG(INFO) << "Creating EventLoop object in thread #" << threadId_;
    if(loopInThisThread != nullptr) {
        LOG(FATAL) << "An EventLoop object already exists in this thread!";
    }
    loopInThisThread = this; // 将这个EventLoop对象保存在线程私有的变量中，用于检测是否在同一个线程中重复创建EventLoop

    // 将用于唤醒的wakekupChannel_注册到当前的EventLoop
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleWakeUp, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    DLOG(INFO) << "Destroying EventLoop object in thread #" << threadId_;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();

    ChannelList activeChannels;

    looping_ = true;
    quit_ = false;    // FIXME 如果这句还没执行，其他线程调用quit()函数的行为是无效的
    DLOG(INFO) << "EventLoop::loop() start looping";
    while(!quit_) {
        activeChannels.clear();
        // I/O多路复用检测事件发生
        TimeStamp now = poller_->poll(kPollTimeMs, activeChannels);

        // 开始处理channel上的事件
        eventHandling_ = true;
        for(ChannelPtr channel : activeChannels) {
            // 调用每个channel的handleEvent函数处理事件
            channel->handleEvent(now);
        }
        // 事件处理结束
        eventHandling_ = false;

        // 处理任务队列中的task
        doPendingFunctors();
    }
    looping_ = false;
    DLOG(INFO) << "EventLoop::loop() end looping";
}

void EventLoop::quit() {
    quit_ = true;
    // 如果不是在IO线程中，则需要唤醒IO线程，否则对quit_的检测将会延时到下一个事件发生或超时后进行
    if(!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor task) {
    if(isInLoopThread()) {
        // 如果在IO线程中调用runInLoop()，则直接同步执行task
        if(task) {
            task();
        }
    } else {
        // 如果在其他线程中调用runInLoop()，则将task加入任务队列
        queueInLoop(std::move(task));
    }
}

void EventLoop::queueInLoop(Functor task) {
    {
        MutexLockGuard lock(mutex_);
        // 将task加入任务队列
        pendingFunctors_.push_back(std::move(task));
    }

    // 判断是否需要唤醒loop，两种情况需要唤醒：
    // 1. 在非IO线程中调用了queueInLoop()
    // 2. 在执行任务队列中的task时
    if(!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

void EventLoop::wakeup() {
    int64_t one = 1;
    int nBytes = ::write(wakeupFd_, &one, sizeof(one));
    if(nBytes == -1) {
        LOG(FATAL) << "Something wrong when call write(), the errno is " << errno << "(" << strerror(errno) << ")";
    } else if(nBytes != sizeof(one)) {
        LOG(WARNING) << "EventLoop::wakeup() writes " << nBytes << " bytes instead of " << sizeof(one);
    }
}

void EventLoop::updateChannel(ChannelPtr channel) {
    assertInLoopThread();
    assert(channel->ownerLoop() == this);

    // 对poller的操作仅限在IO线程内进行
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(ChannelPtr channel) {
    assertInLoopThread();
    assert(channel->ownerLoop() == this);

    // 对poller的操作仅限在IO线程内进行
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(ChannelPtr channel) const {
    assertInLoopThread();
    assert(channel->ownerLoop() == this);

    // 对poller的操作仅限在IO线程内进行
    return poller_->hasChannel(channel);
}

void EventLoop::assertInLoopThread() const {
    assert(isInLoopThread());
}

bool EventLoop::isInLoopThread() const {
    return CurrentThread::tid() == threadId_;
}

void EventLoop::handleWakeUp() {
    int64_t one;
    int nBytes = ::read(wakeupFd_, &one, sizeof(one));
    if(nBytes == -1) {
        LOG(FATAL) << "Something wrong when call read(), the errno is " << errno << "(" << strerror(errno) << ")";
    } else if(nBytes != sizeof(one)) {
        LOG(WARNING) << "EventLoop::handleWakeUp() reads " << nBytes << " bytes instead of " << sizeof(one);
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        MutexLockGuard lock(mutex_);
        // 将pengdingFunctors_中的元素交换到局部的functors中，减小临界区
        functors.swap(pendingFunctors_);
    }

    // functors是局部变量，不与其他线程共享，因此不需要加锁
    for(Functor task : functors) {
        if(task) {
            task();
        }
    }

    callingPendingFunctors_ = false;
}
