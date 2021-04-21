#include "ThreadPool.h"

ThreadPool::ThreadPool(const std::string & name)
    : mutex_()
    , notEmpty_(mutex_)
    , notFull_(mutex_)
    , maxQueueSize_(0)
    , initCallback_([](){})
    , running_(false)
    , name_(name) {
}

ThreadPool::~ThreadPool() {
    if(running_) {
        stop();
    }
}

void ThreadPool::setMaxQueueSize(int size) {
    maxQueueSize_ = size;
}

int ThreadPool::queueSize() {
    MutexLockGuard lock(mutex_);
    return tasks_.size();
}


void ThreadPool::setThreadInitCallback(ThreadInitCallback callback) {
    initCallback_ = callback ? callback : []{};
}


const std::string & ThreadPool::name() const {
    return name_;
}


void ThreadPool::start(int numThread) {
    running_ = true;

    // 如果子线程数量为0，则直接在当前线程中操作
    if(numThread == 0) {
        initCallback_();
        return ;
    }

    pool_.reserve(numThread);
    for(int i = 0; i < numThread; ++i) {
        char buf[32];
        snprintf(buf, 32, "PoolThread#%d", i);
        pool_.emplace_back(new Thread(std::bind(&ThreadPool::runInThread, this), buf));
        pool_.back()->start();
    }
}

void ThreadPool::stop() {
    {
        MutexLockGuard lock(mutex_);
        running_ = false;    // ??? running_ = false要放在notifyAll()之前可以理解，但是为什么要放在这里，而不是上面？
        // 防止子线程一直在等待获取任务，从而导致子线程无法回收
        notEmpty_.notifyAll();
    }
    // 子线程会将当前任务完成后才退出，但不保证任务队列中的任务全部完成
    for(auto & thread : pool_) {
        thread->join();
    }
}


void ThreadPool::run(Task task) {
    // task为空，则忽略
    if(!task) {
        return ;
    }

    // 如果子线程数量为0，则直接在当前线程中执行
    if(pool_.empty()) {
        task();
        return ;
    }

    // 否则将任务加入任务队列
    {
        MutexLockGuard lock(mutex_);
        // 如果队列满了，则等待队列变为非满（队列最大大小设为0则不受限制）
        while(maxQueueSize_ > 0 && tasks_.size() >= maxQueueSize_) {
            notFull_.wait();
        }
        tasks_.push(task);
        // 任务入队之后，队列变为非空，唤醒一个线程来处理
        notEmpty_.notify();
    }
}

ThreadPool::Task ThreadPool::take() {
    MutexLockGuard lock(mutex_);
    // 等待任务队列非空（在running_ == true的前提下）
    while(tasks_.empty() && running_) {
        notEmpty_.wait();
    }
    Task task;
    // 如果上面的循环是因为running_ = false而退出的，则队列可能为空，直接读取会发生异常
    // 调用stop()的时候会将running_变为false，本意是为了唤醒正在等待的线程，让它们自动结束
    if(!tasks_.empty()) {
        task = tasks_.front();
        tasks_.pop();
        // maxQueueSize_为0的时候代表队列大小不受限制，此时notFull_并未使用
        if(maxQueueSize_ > 0) {
            notFull_.notify();
        }
    }
    return task;
}

void ThreadPool::runInThread() {
    try {
        initCallback_();
        // 调用stop()时，running_变为false，同时通知所有线程进行一次读队列，如果队列为空，则读到的task就是空的，线程直接终止
        // 如果队列里还有任务没处理，则从队列中拿一个任务出来处理，处理完它后再结束
        // 但是不能保证队列里的所有剩余任务都能得到处理
        while(running_) {
            Task task = take();
            // 当调用stop()时，如果队列为空，则take()会返回一个空的task对象，所以要进行判空
            if(task) {
                task();
            }
        }
    } catch(...) {
        throw ;
    }
}