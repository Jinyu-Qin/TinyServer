#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include "Thread.h"
#include <vector>
#include <queue>
#include <memory>

class ThreadPool: public boost::noncopyable {
    using ThreadPtr = std::unique_ptr<Thread>;
public:
    using Task = std::function<void(void)>;
    using ThreadInitCallback = std::function<void(void)>;

    explicit ThreadPool(const std::string & name = "");
    ~ThreadPool();

    // 设置任务队列最大大小，必须在start()之前调用
    // 默认为0，代表无限制
    void setMaxQueueSize(int size);
    int queueSize();

    // 回调函数会在线程开始执行（初始化）的时候调用，必须在start()之前调用
    void setThreadInitCallback(ThreadInitCallback callback);

    const std::string & name() const;

    void start(int numThread);
    void stop();

    void run(Task task);

private:
    Task take();
    void runInThread();

    MutexLock mutex_;
    Condition notEmpty_;
    Condition notFull_;

    std::queue<Task> tasks_;
    int maxQueueSize_;

    std::vector<ThreadPtr> pool_;
    ThreadInitCallback initCallback_;
    bool running_;

    std::string name_;
};

#endif //__THREADPOOL_H__
