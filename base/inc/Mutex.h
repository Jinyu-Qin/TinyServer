#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <boost/utility.hpp>
#include <pthread.h>

class MutexLock: public boost::noncopyable {
public:
    MutexLock();
    ~MutexLock();

    void lock();
    void unlock();
    pthread_mutex_t * get();

private:
    pthread_mutex_t mutex_;
};

class MutexLockGuard: public boost::noncopyable {
public:
    MutexLockGuard(MutexLock & mutex);
    ~MutexLockGuard();

private:
    MutexLock & mutex_;
};

#endif //__MUTEX_H__
