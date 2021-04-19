#ifndef __CONDITION_H__
#define __CONDITION_H__

#include "Mutex.h"

class Condition: public boost::noncopyable {
public:
    Condition(MutexLock & mutex);
    ~Condition();

    void wait();
    void notify();
    void notifyAll();

    // 超时返回true，否则返回false
    bool waitForSeconds(int second);

private:
    MutexLock & mutex_;
    pthread_cond_t cond_;
};

#endif //__CONDITION_H__
