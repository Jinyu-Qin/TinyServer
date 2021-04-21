#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include <boost/utility.hpp>
#include <functional>

class EventLoop;

class Channel: public boost::noncopyable {
public:
    using Callback = std::function<void(void)>;
    Channel(EventLoop * loop, int fd);
    ~Channel();

    int fd() const;
    int events() const;
    void setRevents(int revents);

    void setReadCallback(Callback callback);
    void setWriteCallback(Callback callback);
    void setCloseCallback(Callback callback);
    void setErrorCallback(Callback callback);

    void enableReading(bool enable = true);
    void enableWriting(bool enable = true);
    bool isReading() const;
    bool isWriting() const;

    void handleEvent();
    
    void remove();

private:
    void update();

    int fd_;
    int events_;
    int revents_;
    
    EventLoop * loop_;

    Callback readCallback_;
    Callback writeCallback_;
    Callback closeCallback_;
    Callback errorCallback_;
};


#endif //__CHANNEL_H__
