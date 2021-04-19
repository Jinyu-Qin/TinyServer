#include "Thread.h"
#include <utility>
#include <cstdio>
#include "CurrentThread.h"

static void * startThread(void * args);

// ThreadData封装了线程运行需要的参数信息
class ThreadData: public boost::noncopyable {
public:
    friend void * startThread(void * args);
    using ThreadFunc = typename Thread::ThreadFunc;

    ThreadData(ThreadFunc func, const std::string & name, pid_t * tid, CountDownLatch * latch)
        : func_(std::move(func))
        , name_(name)
        , tid_(tid)
        , latch_(latch) {
    }

    ~ThreadData() {
    }

private:
    void runInThread() {
        *tid_ = CurrentThread::tid();
        tid_ = nullptr;
        latch_->countDown();
        latch_ = nullptr;
        CurrentThread::setName(name_.c_str());

        try {
            func_();
        } catch(...) {
            abort();
        }
    }

    ThreadFunc func_;
    std::string name_;
    pid_t * tid_;
    CountDownLatch * latch_;
};

static void * startThread(void * args) {
    auto data = static_cast<ThreadData *>(args);
    data->runInThread();
    delete data;
    data = nullptr;
    pthread_exit(nullptr);
}

Thread::Thread(ThreadFunc func, const std::string & name)
    : started_(false)
    , joined_(false)
    , pthread_(0)
    , tid_(0)
    , func_(std::move(func))
    , name_(name)
    , latch_(1) {
    int num;    // 保存线程编号
    {
        // 增加线程计数
        MutexLockGuard lock(mutex_);
        num = numThread_;
        ++numThread_;
    }

    // 设置默认线程名
    if(name_.empty()) {
        char buf[32];
        snprintf(buf, 32, "Thread-%d", num);
        name_.assign(buf);
    }
}

Thread::~Thread() {
    // 如果销毁时，线程还在运行，则将线程脱离，线程结束后将自动回收
    if(started_ && !joined_) {
        pthread_detach(pthread_);
    }
}

void Thread::start() {
    started_ = true;

    auto data = new ThreadData(func_, name_, &tid_, &latch_);
    int ret = pthread_create(&pthread_, nullptr, startThread, static_cast<void *>(data));
    if(ret != 0) {
        // 线程创建出错
        delete data;
        data = nullptr;
        started_ = false;
    } else {
        // 等待线程开始运行并准备好之后才返回
        latch_.wait();
    }
}

int Thread::join() {
    joined_ = true;
    int ret = pthread_join(pthread_, nullptr);
    return ret;
}

bool Thread::started() const {
    return started_;
}

pid_t Thread::tid() const {
    return tid_;
}

const std::string & Thread::name() const {
    return name_;
}

MutexLock Thread::mutex_;
int Thread::numThread_ = 0;