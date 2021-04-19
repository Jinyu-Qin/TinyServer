#ifndef __THREAD_H__
#define __THREAD_H__

#include <functional>
#include <string>
#include <boost/utility.hpp>
#include <unistd.h>
#include "CountDownLatch.h"

class Thread: public boost::noncopyable {
public:
    using ThreadFunc = std::function<void(void)>;
    explicit Thread(ThreadFunc func, const std::string & name = std::string());
    ~Thread();

    void start();
    // 返回pthread_join()的返回值
    int join();
    bool started() const;
    pid_t tid() const;
    const std::string & name() const;

private:
    bool started_;
    bool joined_;
    pthread_t pthread_;
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;
    CountDownLatch latch_;

    static MutexLock mutex_;
    static int numThread_;
};

#endif //__THREAD_H__
