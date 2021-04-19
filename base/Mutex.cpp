#include "Mutex.h"

MutexLock::MutexLock() {
    pthread_mutex_init(&mutex_, nullptr);
}

MutexLock::~MutexLock() {
    // 销毁一个已经加锁的mutex将产生未定义行为
    // pthread_mutex_lock(&mutex_);
    pthread_mutex_destroy(&mutex_);
}

void MutexLock::lock() {
    pthread_mutex_lock(&mutex_);
}

void MutexLock::unlock() {
    pthread_mutex_unlock(&mutex_);
}

pthread_mutex_t * MutexLock::get() {
    return &mutex_;
}

MutexLockGuard::MutexLockGuard(MutexLock & mutex)
    : mutex_(mutex) {
    // 构建一个MutexLockGuard对象时自动加锁
    mutex_.lock();
}

MutexLockGuard::~MutexLockGuard() {
    // 当MutexLockGuard对象被销毁时，自动解锁
    mutex_.unlock();
}
