#include "Condition.h"
#include <errno.h>

Condition::Condition(MutexLock & mutex)
    : mutex_(mutex) {
    pthread_cond_init(&cond_, nullptr);
}

Condition::~Condition() {
    pthread_cond_destroy(&cond_);
}

void Condition::wait() {
    pthread_cond_wait(&cond_, mutex_.get());
}

void Condition::notify() {
    pthread_cond_signal(&cond_);
}

void Condition::notifyAll() {
    pthread_cond_broadcast(&cond_);
}

bool Condition::waitForSeconds(int second) {
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);
    abstime.tv_sec += static_cast<time_t>(second);
    return ETIMEDOUT == pthread_cond_timedwait(&cond_, mutex_.get(), &abstime);
}
