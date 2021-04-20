#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <boost/utility.hpp>
#include <sys/socket.h>

class ServerSocket: public boost::noncopyable {
public:
    ServerSocket();
    ~ServerSocket();

    void setReuseAddress(bool enable = true);
    void setNonblocking(bool enable = true);
    
    void bind(const char * addr, int port);
    void listen(int backlog = SOMAXCONN);
    int accept();
    void close();

    bool closed() const;
    int fd() const;

private:
    int sockfd_;
    bool opened_;
};

#endif //__SOCKET_H__
