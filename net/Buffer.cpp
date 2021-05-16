#include "Buffer.h"
#include <cassert>
#include <algorithm>
#include <unistd.h>
#include <sys/uio.h>

Buffer::Buffer(size_type initialSize)
    : buffer_(initialSize)
    , readIndex_(0)
    , writeIndex_(0) {
}

Buffer::~Buffer() {
}

Buffer::size_type Buffer::readableSize() {
    return writeBegin() - readBegin();
}

Buffer::size_type Buffer::writableSize() {
    return buffer_.data() + buffer_.size() - writeBegin();
}

char * Buffer::readBegin() {
    return buffer_.data() + readIndex_;
}

char * Buffer::writeBegin() {
    return buffer_.data() + writeIndex_;
}

void Buffer::hasRead(size_type size) {
    assert(readBegin() + size <= writeBegin());

    readIndex_ += size;

    if(readBegin() == writeBegin()) {
        readIndex_ = 0;
        writeIndex_ = 0;
    }
}

void Buffer::hasWritten(size_type size) {
    assert(writeBegin() + size <= buffer_.data() + buffer_.size());

    writeIndex_ += size;
}

Buffer::size_type Buffer::readIntoFd(int fd, size_type size) {
    assert(size >= -1);
    assert(readableSize() >= size);

    return ::write(fd, readBegin(), size == -1 ? readableSize() : size);
}

Buffer::size_type Buffer::writeFromFd(int fd, size_type size) {
    assert(size >= -1);

    if(size != -1 && writableSize() >= size) {
        size_type nBytes = ::read(fd, writeBegin(), size);
        if(nBytes > 0) {
            hasWritten(nBytes);
        }
        return nBytes;
    } else {
        char buf[size == -1 ? 65535 : size - writableSize()];

        struct iovec iov[2];
        iov[0].iov_base = writeBegin();
        iov[0].iov_len = writableSize();
        iov[1].iov_base = buf;
        iov[1].iov_len = sizeof(buf);

        size_type nBytes = ::readv(fd, iov, 2);
        if(nBytes > 0) {
            if(nBytes <= writableSize()) {
                hasWritten(nBytes);
            } else {
                int writable = writableSize();
                hasWritten(writable);
                write(buf, nBytes - writable);
            }
        }
        return nBytes;
    }
}

Buffer::size_type Buffer::read(char * buf, size_type size) {
    assert(size >= 0);
    assert(readableSize() >= size);

    return std::copy(readBegin(), readBegin() + size, buf) - buf;
}

Buffer::size_type Buffer::write(const char * buf, size_type size) {
    assert(size >= 0);

    ensure(size);
    size_type nBytes = std::copy(buf, buf + size, writeBegin()) - writeBegin();
    hasWritten(nBytes);

    return nBytes;
}

void Buffer::ensure(size_type size) {
    assert(size >= 0);
    
    if(writableSize() < size) {
        int newsize = buffer_.size();
        do {
            newsize *= 2;
        } while(writableSize() < size);
        buffer_.resize(newsize);
    }
}
