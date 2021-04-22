#include "Buffer.h"

Buffer::Buffer(int capacity)
    : buffer_(new char[capacity])
    , head_(0)
    , tail_(0)
    , capacity_(capacity)
    , size_(0) {
}

Buffer::~Buffer() {
}

int Buffer::size() const {
    return size_;
}

int Buffer::continuousSize() const {
    return std::min(capacity_ - head_, size_);
}

int Buffer::capacity() const {
    return capacity_;
}

int Buffer::available() const {
    return capacity_ - size_;
}

int Buffer::continuousAvailable() const {
    if(full()) {
        return 0;
    }

    return tail_ >= head_ ? capacity_ - tail_ : head_ - tail_;
}

bool Buffer::full() const {
    return size_ == capacity_;
}

bool Buffer::empty() const {
    return size_ == 0;
}

char * Buffer::readBegin() const {
    return &buffer_[head_];
}

char * Buffer::writeBegin() const {
    return &buffer_[tail_];
}

int Buffer::readSeek(int offset) {
    if(offset > size_) {
        offset = size_;
    }

    head_ = (head_ + offset) % capacity_;
    size_ -= offset;

    return offset;
}

int Buffer::writeSeek(int offset) {
    if(offset > available()) {
        offset = available();
    }

    tail_ = (tail_ + offset) % capacity_;
    size_ += offset;

    return offset;
}

int Buffer::read(const char *& buf, int n) {
    if(n > size_) {
        n = size_;
    }

    if(head_ + n > capacity_) {
        n = capacity_ - head_;
    }

    buf = &buffer_[head_];
    head_ = (head_ + n) % capacity_;
    size_ -= n;

    return n;
}

int Buffer::write(const char * buf, int n) {
    if(n > available()) {
        n = available();
    }

    int i = 0;
    while(i < n) {
        buffer_[tail_] = buf[i++];
        tail_ = (tail_ + n) % capacity_;
    }
    size_ += n;

    return n;
}
