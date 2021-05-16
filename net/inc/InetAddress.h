#ifndef __INETADDRESS_H__
#define __INETADDRESS_H__

#include <netinet/in.h>

#include <string>
#include <ostream>

class InetAddress {
public:
    InetAddress(const std::string & ip, uint16_t port);
    explicit InetAddress(uint16_t port);
    InetAddress(const struct sockaddr_in & addr);
    ~InetAddress();

    // 获取协议类型
    sa_family_t family() const;
    // 获取IP地址
    std::string ip() const;
    // 获取端口号
    uint16_t port() const;
    // 转换为"ip:port"形式的字符串
    operator std::string() const;

private:
    struct sockaddr_in addr_;
};

std::ostream & operator<<(std::ostream & os, const InetAddress & addr);

#endif //__INETADDRESS_H__
