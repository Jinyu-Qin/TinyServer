#ifndef __COUNTDOWNLATCH_H__
#define __COUNTDOWNLATCH_H__

#include <boost/utility.hpp>
#include "Condition.h"

class CountDownLatch: public boost::noncopyable {
public:
    CountDownLatch(int initialCount);
    ~CountDownLatch();

    // 等待count_变为0
    void wait();
    // 减少计数
    void countDown();
    int getCount();

private:
    MutexLock mutex_;
    Condition cond_;
    int count_;
};

#endif //__COUNTDOWNLATCH_H__
