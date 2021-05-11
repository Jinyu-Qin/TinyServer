#include "TimeStamp.h"
#include <sys/time.h>
#include <cstring>

TimeStamp::TimeStamp()
    : TimeStamp(static_cast<int64_t>(0)) {
}

TimeStamp::TimeStamp(int64_t microseconds)
    : microseconds_(microseconds) {
}

TimeStamp::~TimeStamp() {
}

int64_t TimeStamp::seconds() const {
    return microseconds_ / kMicroSecondsPerSecond;
}

int64_t TimeStamp::microseconds() const {
    return microseconds_;
}

bool TimeStamp::valid() const {
    return microseconds_ > 0;
}

TimeStamp TimeStamp::now() {
    struct timeval tv;
    memset(&tv, 0, sizeof(tv));

    gettimeofday(&tv, NULL);

    int64_t microseconds = tv.tv_sec * kMicroSecondsPerSecond;
    microseconds += tv.tv_usec;

    return TimeStamp(microseconds);
}

TimeStamp TimeStamp::createWithSeconds(int64_t seconds) {
    return TimeStamp(seconds * kMicroSecondsPerSecond);
}

bool operator<(const TimeStamp & lhs, const TimeStamp & rhs) {
    return lhs.microseconds() < rhs.microseconds();
}

bool operator<=(const TimeStamp & lhs, const TimeStamp & rhs) {
    return lhs.microseconds() <= rhs.microseconds();
}

bool operator>(const TimeStamp & lhs, const TimeStamp & rhs) {
    return lhs.microseconds() > rhs.microseconds();
}

bool operator>=(const TimeStamp & lhs, const TimeStamp & rhs) {
    return lhs.microseconds() >= rhs.microseconds();
}

bool operator==(const TimeStamp & lhs, const TimeStamp & rhs) {
    return lhs.microseconds() == rhs.microseconds();
}

bool operator!=(const TimeStamp & lhs, const TimeStamp & rhs) {
    return !(lhs == rhs);
}

TimeStamp operator-(const TimeStamp & lhs, const TimeStamp & rhs) {
    return TimeStamp(lhs.microseconds() - rhs.microseconds());
}

TimeStamp operator+(const TimeStamp & lhs, const TimeStamp & rhs) {
    return TimeStamp(lhs.microseconds() + rhs.microseconds());
}
