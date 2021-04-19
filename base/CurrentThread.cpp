#include "CurrentThread.h"
#include <cstdio>
#include <sys/syscall.h>
#include <pthread.h>

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define THREAD_STRING_LEN_MAX   (32)

namespace CurrentThread {

__thread pid_t cachedTid = 0;
__thread char tidStr[THREAD_STRING_LEN_MAX] = "0";
__thread int tidStrLen = 1;
__thread const char * threadName = "default";

static void afterFork();
static pid_t gettid();

// 用于初始化主线程的一些信息和设置
class ThreadNameInitializer {
public:
    ThreadNameInitializer() {
        cachedTid = 0;
        setName("main");
        tid();
        // 设置fork执行前后的回调函数
        pthread_atfork(nullptr, nullptr, afterFork);
    }
};

// 获取线程tid
static pid_t gettid() {
    return static_cast<pid_t>(syscall(SYS_gettid));
}

// 在fork成功后执行（子进程中）
static void afterFork() {
    cachedTid = 0;
    setName("main");
    tid();
}

// 用于初始化主线程的名字，在main()之前执行
ThreadNameInitializer initializer;

pid_t tid() {
    if(unlikely(cachedTid == 0)) {
        cachedTid = gettid();
        tidStrLen = snprintf(tidStr, THREAD_STRING_LEN_MAX, "%d", cachedTid);
    }
    return cachedTid;
}


const char * tidString() {
    return tidStr;
}

int tidStringLength() {
    return tidStrLen;
}

const char * name() {
    return threadName;
}

void setName(const char * name) {
    threadName = name;
}

bool isMainThread() {
    return tid() == getpid();
}

}