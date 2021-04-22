#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <boost/utility.hpp>
#include <memory>

class Buffer: public boost::noncopyable {
public:
    explicit Buffer(int capacity = 8192);
    ~Buffer();

    int size() const;
    int continuousSize() const;
    int capacity() const;
    int available() const;
    int continuousAvailable() const;
    
    bool full() const;
    bool empty() const;

    char * readBegin() const;
    char * writeBegin() const;

    int readSeek(int offset);
    int writeSeek(int offset);

    int read(const char *& buf, int n);
    int write(const char * buf, int n);

private:
    std::unique_ptr<char[]> buffer_;
    int head_;
    int tail_;
    int capacity_;
    int size_;
};

#endif //__BUFFER_H__
