#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <boost/noncopyable.hpp>
#include <vector>

class Buffer: public boost::noncopyable {
public:
    using size_type = ssize_t;

    explicit Buffer(size_type initialSize = kBufferInitialSize);
    ~Buffer();

    size_type readableSize();
    size_type writableSize();

    char * readBegin();
    char * writeBegin();

    void hasRead(size_type size);
    void hasWritten(size_type size);

    size_type readIntoFd(int fd, size_type size = -1);
    size_type writeFromFd(int fd, size_type size = -1);

    size_type read(char * buf, size_type size);
    size_type write(const char * buf, size_type size);
    void ensure(size_type size);

private:

    std::vector<char> buffer_;
    size_type readIndex_;
    size_type writeIndex_;

    static constexpr size_type kBufferInitialSize = 8192;
};

#endif //__BUFFER_H__
