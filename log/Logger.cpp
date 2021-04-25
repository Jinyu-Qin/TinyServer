#include "Logger.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include <errno.h>
#include <cstdio>
#include <cstdarg>
#include "CurrentThread.h"

Logger::Logger(LogLevel level, int bufferSize)
    : thread_(new Thread(std::bind(&Logger::ThreadFunc, this), "LoggingThread"))
    , level_(level)
    , running_(false)
    , currentBuffer_(new Buffer(bufferSize))
    , nextBuffer_(new Buffer(bufferSize))
    , buffers_()
    , bufferSize_(bufferSize)
    , mutex_()
    , cond_(mutex_)
    , latch_(1) {
    buffers_.reserve(16);
    thread_->start();
    latch_.wait();
}

Logger::~Logger() {
    running_ = false;
    cond_.notify();
    thread_->join();
}

void Logger::log(LogLevel level, const char * format, ...) {
    if(level > level_) {
        return ;
    }
    
    va_list ap;
    va_start(ap, format);
    int len = snprintf(buf_, sizeof(buf_), "[%s][%d][%s]\t", logLevelName_[static_cast<int>(level)], CurrentThread::tid(), CurrentThread::name());
    len += vsnprintf(buf_ + len, sizeof(buf_) - len, format, ap);
    if(len > sizeof(buf_) - 3) {
        len = sizeof(buf_) - 3;
    }
    buf_[len++] = '\r';
    buf_[len++] = '\n';
    buf_[len++] = '\0';
    log(buf_, len);
    va_end(ap);
}

void Logger::log(const char * msg, int len) {
    MutexLockGuard lock(mutex_);
    if(currentBuffer_->available() < len) {
        buffers_.emplace_back(std::move(currentBuffer_));
        if(nextBuffer_) {
            currentBuffer_ = std::move(nextBuffer_);
        } else {
            currentBuffer_.reset(new Buffer(bufferSize_));
        }
    }
    currentBuffer_->write(msg, len);
    cond_.notify();
}

void Logger::ThreadFunc() {
    running_ = true;
    latch_.countDown();
    std::vector<BufferPtr> buffers;
    buffers.reserve(16);
    BufferPtr curBuffer(new Buffer(bufferSize_));

    while(running_) {
        {
            MutexLockGuard lock(mutex_);
            while(running_ && buffers_.empty() && currentBuffer_->empty()) {
                cond_.wait();
            }
            if(!running_) {
                break;
            }
            buffers.swap(buffers_);
            curBuffer.swap(currentBuffer_);
        }

        int n = buffers.size();
        for(int i = 0; i < n; ++i) {
            while(!buffers[i]->empty()) {
                int len = buffers[i]->continuousSize();
                int ret = write(STDOUT_FILENO, buffers[i]->readBegin(), len);
                if(ret == -1) {
                    if(errno != EAGAIN) {
                        throw std::runtime_error("couldn't write some bytes to STDOUT_FILENO");
                    }
                } else {
                    buffers[i]->readSeek(ret);
                }
            }
        }
        while(!curBuffer->empty()) {
            int len = curBuffer->continuousSize();
            int ret = write(STDOUT_FILENO, curBuffer->readBegin(), len);
            if(ret == -1) {
                if(errno != EAGAIN) {
                    throw std::runtime_error("couldn't write some bytes to STDOUT_FILENO");
                }
            } else {
                curBuffer->readSeek(ret);
            }
        }

        if(buffers.size() > 0) {
            {
                MutexLockGuard lock(mutex_);
                if(!nextBuffer_) {
                    nextBuffer_ = std::move(buffers.back());
                }
            }
        }
        buffers.clear();
    }

    // 输出剩余缓冲区中的内容
    {
        MutexLockGuard lock(mutex_);
        buffers.swap(buffers_);
        curBuffer.swap(currentBuffer_);
    }
    int n = buffers.size();
    for(int i = 0; i < n; ++i) {
        while(!buffers[i]->empty()) {
            int len = buffers[i]->continuousSize();
            int ret = write(STDOUT_FILENO, buffers[i]->readBegin(), len);
            if(ret == -1) {
                if(errno != EAGAIN) {
                    throw std::runtime_error("couldn't write some bytes to STDOUT_FILENO");
                }
            } else {
                buffers[i]->readSeek(ret);
            }
        }
    }
    while(!curBuffer->empty()) {
        int len = curBuffer->continuousSize();
        int ret = write(STDOUT_FILENO, curBuffer->readBegin(), len);
        if(ret == -1) {
            if(errno != EAGAIN) {
                throw std::runtime_error("couldn't write some bytes to STDOUT_FILENO");
            }
        } else {
            curBuffer->readSeek(ret);
        }
    }
}

MutexLock Logger::mutexCreate_{};
std::weak_ptr<Logger> Logger::logger_{};
__thread char Logger::buf_[512]{};
char Logger::logLevelName_[5][16] = {
    "NONE", "ERROR", "WARNING", "INFO", "DEBUG"
};

Logger::LoggerPtr Logger::getLogger() {
    auto p = logger_.lock();
    if(!p) {
        MutexLockGuard lock(mutexCreate_);
        p = logger_.lock();
        if(!p) {
            p.reset(new Logger(DEBUG, 2048));
            logger_ = p;
        }
    }
    return p;
}
