#include <iostream>
#include <string>
#include <glog/logging.h>
#include <cassert>
#include <signal.h>
#include "EventLoop.h"
#include "EchoServer.h"
#include "InetAddress.h"
#include "HttpServer.h"

EventLoop * mainLoop = nullptr;

int main(int argc, char * argv[]) {
    EventLoop loop;
    mainLoop = &loop;

    HttpServer httpServer(mainLoop, "HttpServer", InetAddress("0.0.0.0", 2222), "/home/qinjinyu/workspace/TinyServer/www");
    httpServer.start();
    mainLoop->loop();

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

class PipeSignalInitializer {
public:
    PipeSignalInitializer() {
        signal(SIGPIPE, SIG_IGN);
    }
};

class DaemonInitializer {
public:
    DaemonInitializer() {
        daemon(1, 0);
    }
};

GoogleLoggingInitializer glogInit;
InterruptSignalInitializer intSignalInit;
PipeSignalInitializer pipeSignalInit;
DaemonInitializer daemonInit;
