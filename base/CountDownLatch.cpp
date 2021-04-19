#include "CountDownLatch.h"

CountDownLatch::CountDownLatch(int initialCount)
    : mutex_()
    , cond_(mutex_)
    , count_(initialCount) {
}

CountDownLatch::~CountDownLatch() {
}


void CountDownLatch::wait() {
    MutexLockGuard lock(mutex_);
    while(count_ > 0) {
        cond_.wait();
    }
}

void CountDownLatch::countDown() {
    MutexLockGuard lock(mutex_);
    --count_;
    if(count_ == 0) {
        cond_.notifyAll();
    }
}


int CountDownLatch::getCount() {
    MutexLockGuard lock(mutex_);
    return count_;
}
