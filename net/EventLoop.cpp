#include "EventLoop.h"
#include <stdexcept>
#include <sys/eventfd.h>
#include <fcntl.h>
#include "CurrentThread.h"
#include "Channel.h"

EventLoop::EventLoop()
    : looping_(false)
    , taskHandling_(false)
    , eventHandling_(false)
    , tid_(CurrentThread::tid())
    , poller_(Poller::createDefaultPoller(this))
    , wakeupChannel_(new Channel(this, createEventFd()))
    , tasks_()
    , mutex_() {
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleWakeupRead, this));
    wakeupChannel_->enableReading(true);
}

EventLoop::~EventLoop() {
    wakeupChannel_->enableReading(false);
    wakeupChannel_->remove();
    close(wakeupChannel_->fd());
}

void EventLoop::loop() {
    std::vector<Channel *> activeChannels;
    looping_ = true;
    while(looping_) {
        activeChannels.clear();
        poller_->poll(10000, activeChannels);
        
        eventHandling_ = true;
        for(Channel * channel : activeChannels) {
            channel->handleEvent();
        }
        eventHandling_ = false;

        taskHandling_ = true;
        decltype(tasks_) tasks;
        {
            MutexLockGuard lock(mutex_);
            // 交换到局部变量，减少加锁时间
            tasks.swap(tasks_);
        }
        for(Task & task : tasks) {
            task();
        }
        taskHandling_ = false;
    }
    looping_ = false;
}

void EventLoop::quit() {
    looping_ = false;
}

void EventLoop::updateChannel(Channel * channel) {
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel * channel) {
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel * channel) {
    return poller_->hasChannel(channel);
}

bool EventLoop::taskHandling() const {
    // taskHandling_是基本类型，只对其进行读操作是原子性的
    return taskHandling_;
}

bool EventLoop::eventHandling() const {
    // eventHandling_是基本类型，只对其进行读操作是原子性的
    return eventHandling_;
}

bool EventLoop::isInLoopThread() const {
    return tid_ == CurrentThread::tid();
}

pid_t EventLoop::tid() const {
    return tid_;
}

void EventLoop::runInLoop(Task task) {
    if(!task) {
        return ;
    }

    if(isInLoopThread()) {
        task();
    } else {
        queueInLoop(std::move(task));
    }
}

void EventLoop::queueInLoop(Task task) {
    {
        MutexLockGuard lock(mutex_);
        tasks_.push_back(std::move(task));
    }
    // 在其他线程将任务入队后，需要唤醒线程执行,否则将等到下一次poller_检测到事件发生后执行
    // 如果在本线程调用queueInLoop()，则意味着要么在事件处理函数里调用了，要么在task_中的任务中调用了
    // 在执行任务时，会将tasks_中的任务移动到一份拷贝中，然后执行拷贝中的任务，而这里是将任务添加到tasks_中，如果不唤醒线程，则任务将不能即时得到执行
    // 而在事件处理过程中，tasks_还没有拷贝，是直接将任务添加到tasks_中的，所以可以得到即时处理，所以不需要唤醒
    if(!isInLoopThread() || taskHandling_) {
        wakeup();
    }
}

int EventLoop::queueSize() {
    MutexLockGuard lock(mutex_);
    return tasks_.size();
}

void EventLoop::wakeup() {
    char buf[8] = {1};
    int ret = write(wakeupChannel_->fd(), buf, sizeof(buf));
    if(ret != sizeof(buf)) {
        throw std::runtime_error("couldn't write to eventfd");
    }
}

int EventLoop::createEventFd() {
    int ret = eventfd(0, O_NONBLOCK | O_CLOEXEC);
    if(ret == -1) {
        throw std::runtime_error("couldn't create eventfd");
    }
    return ret;
}

void EventLoop::handleWakeupRead() {
    char buf[8] = {};
    int ret = read(wakeupChannel_->fd(), buf, sizeof(buf));
    if(ret != sizeof(buf)) {
        throw std::runtime_error("couldn't read from eventfd");
    }
}