#ifndef __EVENTLOOPTHREADPOOL_H__
#define __EVENTLOOPTHREADPOOL_H__

#include <boost/utility.hpp>
#include <functional>
#include <string>
#include <vector>
#include <memory>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool: public boost::noncopyable {
public:
    using ThreadInitCallback    = std::function<void(EventLoop *)>;

    EventLoopThreadPool(EventLoop * loop, const std::string & name);
    ~EventLoopThreadPool();

    // 设置线程池的大小
    void setThreadNum(int numThreads);
    // 启动线程池
    void start(const ThreadInitCallback & callback = ThreadInitCallback());
    // 获取一个可用的EventLoop
    EventLoop * getNextLoop();
    // 获取所有可用的EventLoop
    std::vector<EventLoop *> getAllLoops();
    // 线程池是否已经启动
    bool started() const;
    // 当前线程池的名称
    const std::string & name() const;

private:
    EventLoop * loop_;
    std::string name_;

    bool started_;
    int numThreads_;
    int next_;

    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
};

#endif //__EVENTLOOPTHREADPOOL_H__
