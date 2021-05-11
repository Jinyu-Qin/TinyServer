#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__

#include <boost/utility.hpp>
#include <cstdint>

class TimeStamp {
public:
    TimeStamp();
    explicit TimeStamp(int64_t microseconds);
    ~TimeStamp();

    int64_t seconds() const;
    int64_t microseconds() const;

    bool valid() const;

    static TimeStamp now();
    static TimeStamp createWithSeconds(int64_t seconds);

    static constexpr int64_t kMicroSecondsPerSecond = 1000 * 1000;
private:
    const int64_t microseconds_;
};

bool operator<(const TimeStamp & lhs, const TimeStamp & rhs);
bool operator<=(const TimeStamp & lhs, const TimeStamp & rhs);
bool operator>(const TimeStamp & lhs, const TimeStamp & rhs);
bool operator>=(const TimeStamp & lhs, const TimeStamp & rhs);
bool operator==(const TimeStamp & lhs, const TimeStamp & rhs);
bool operator!=(const TimeStamp & lhs, const TimeStamp & rhs);
TimeStamp operator-(const TimeStamp & lhs, const TimeStamp & rhs);
TimeStamp operator+(const TimeStamp & lhs, const TimeStamp & rhs);

#endif //__TIMESTAMP_H__
