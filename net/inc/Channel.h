#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include <boost/utility.hpp>
#include <functional>

class EventLoop;
class TimeStamp;

// Channel封装了文件描述符及其相应的事件处理函数
class Channel: public boost::noncopyable {
public:
    using ReadEventCallback = std::function<void(TimeStamp)>;
    using EventCallback     = std::function<void(void)>;

    Channel(EventLoop * loop, int fd);
    ~Channel();

    // 获取Channel所属的EventLoop
    EventLoop * ownerLoop() const;
    // 获取Channel对应的文件描述符
    int fd() const;
    // 获取关注的事件
    int events() const;
    // 设置响应的事件（给Poller用的）
    void setRevents(int revents);

    // 设置可读事件回调函数
    void setReadCallback(ReadEventCallback callback);
    // 设置可写事件回调函数
    void setWriteCallback(EventCallback callback);
    // 设置关闭事件回调函数
    void setCloseCallback(EventCallback callback);
    // 设置错误事件回调函数
    void setErrorCallback(EventCallback callback);

    // 事件处理函数
    void handleEvent(TimeStamp time);

    // 使能可读事件
    void enableReading();
    // 使能可写事件
    void enableWriting();
    // 禁止可读事件
    void disableReading();
    // 禁止可写事件
    void disableWriting();
    // 禁止所有事件
    void disableAll();
    // 查看是否使能可读事件
    bool isReading() const;
    // 查看是否使能可写事件
    bool isWriting() const;

    // 将channel脱离所在的loop
    void remove();

private:
    // 更新channel在loop中的注册信息
    void update();

    EventLoop * loop_;
    int fd_;
    int events_;
    int revents_;

    ReadEventCallback readCallback;
    EventCallback writeCallback;
    EventCallback closeCallback;
    EventCallback errorCallback;

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;
};

#endif //__CHANNEL_H__
