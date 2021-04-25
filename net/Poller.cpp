#include "Poller.h"
#include <cstdlib>
#include <string>
#include "EpollPoller.h"
#include "PollPoller.h"
#include "SelectPoller.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

Poller::Poller(EventLoop * loop)
    : loop_(loop) {
}

Poller::~Poller() {
}

bool Poller::hasChannel(Channel * channel) const {
    auto iter = channels_.find(channel->fd());
    return iter != channels_.cend();
}

EventLoop * Poller::eventLoop() const {
    return loop_;
}

Poller::PollerPtr Poller::createDefaultPoller(EventLoop * loop) {
    char * poller_name = getenv("TINY_SERVER_POLLER");
    Logger::LoggerPtr logger = Logger::getLogger();

    if(poller_name != nullptr) {
        std::string poller(poller_name);
        if(poller == "EPOLL") {
            logger->log(Logger::INFO, "use EPOLL");
            return PollerPtr(new EpollPoller(loop));
        } else if(poller == "POLL") {
            logger->log(Logger::INFO, "use POLL");
            return PollerPtr(new PollPoller(loop));
        } else if(poller == "SELECT") {
            logger->log(Logger::INFO, "use SELECT");
            return PollerPtr(new SelectPoller(loop));
        }
    }
    logger->log(Logger::INFO, "use EPOLL");
    return PollerPtr(new EpollPoller(loop));
}
