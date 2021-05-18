#include <iostream>
#include <string>
#include <glog/logging.h>
#include <cassert>
#include <signal.h>
#include "EventLoop.h"
#include "EchoServer.h"
#include "InetAddress.h"

EventLoop * mainLoop = nullptr;

int main(int argc, char * argv[]) {
    EventLoop loop;
    mainLoop = &loop;

    EchoServer echoServer(mainLoop, "EchoServer", InetAddress("0.0.0.0", 2222));
    echoServer.start();
    mainLoop->loop();

    std::cout << echoServer.name() << " exited!" << std::endl;
    return 0;
}



class GoogleLoggingInitializer {
public:
    GoogleLoggingInitializer() {
        // FIXME 这里改成动态获取程序名
        google::InitGoogleLogging("tinyserver");
        FLAGS_log_dir = "./log";
    }
};

class InterruptSignalInitializer {
public:
    InterruptSignalInitializer() {
        signal(SIGINT, handleInterruptSignal);
    }

private:
    static void handleInterruptSignal(int signo) {
        assert(mainLoop != nullptr);
        assert(signo == SIGINT);

        mainLoop->quit();
    }
};

GoogleLoggingInitializer glogInit;
InterruptSignalInitializer intSignalInit;
