#include "InetAddress.h"
#include <arpa/inet.h>

InetAddress::InetAddress(const std::string & ip, uint16_t port) {
    addr_.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(uint16_t port)
    : InetAddress("127.0.0.1", port) {
}

InetAddress::InetAddress(const struct sockaddr_in & addr)
    : addr_(addr) {
}

InetAddress::~InetAddress() {
}

sa_family_t InetAddress::family() const {
    return addr_.sin_family;
}

std::string InetAddress::ip() const {
    char buf[16];
    return inet_ntop(addr_.sin_family, &addr_.sin_addr, buf, sizeof(buf));
}

uint16_t InetAddress::port() const {
    return ntohs(addr_.sin_port);
}

InetAddress::operator std::string() const {
    std::string s;
    s.reserve(24);
    s.append(ip());
    s.push_back(':');
    s.append(std::to_string(port()));
    return s;
}

InetAddress::operator struct sockaddr_in() const {
    return addr_;
}

std::ostream & operator<<(std::ostream & os, const InetAddress & addr) {
    os << static_cast<std::string>(addr);
    return os;
}
