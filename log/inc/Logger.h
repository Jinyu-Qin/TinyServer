#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <boost/utility.hpp>
#include <memory>
#include <vector>
#include "Mutex.h"
#include "Condition.h"
#include "Thread.h"
#include "Buffer.h"
#include "CountDownLatch.h"

class Logger: public boost::noncopyable {
    using ThreadPtr = std::unique_ptr<Thread>;
    using BufferPtr = std::unique_ptr<Buffer>;
public:
    using LoggerPtr = std::shared_ptr<Logger>;
    ~Logger();

    enum LogLevel {
        NONE = 0,
        ERROR = 1,
        WARNING = 2,
        INFO = 3,
        DEBUG = 4
    };

    void log(LogLevel level, const char * format, ...);

    static LoggerPtr getLogger();

private:
    explicit Logger(LogLevel level = DEBUG, int bufferSize = 16384);
    explicit Logger(const std::string & filename, LogLevel level = DEBUG, int bufferSize = 16384);
    void ThreadFunc();
    void log(const char * msg, int len);

    ThreadPtr thread_;
    bool running_;

    int fd_;
    bool closeFd_;

    LogLevel level_;

    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    std::vector<BufferPtr> buffers_;
    int bufferSize_;

    MutexLock mutex_;
    Condition cond_;
    CountDownLatch latch_;

    static MutexLock mutexCreate_;
    static std::weak_ptr<Logger> logger_;
    static __thread char buf_[512];
    static char logLevelName_[5][16];
};

#endif //__LOGGER_H__
