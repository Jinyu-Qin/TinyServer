#include <iostream>
#include "EventLoop.h"
#include "EchoServer.h"

int main(int argc, char * argv[]) {
    EventLoop loop;
    EchoServer echoServer(&loop);
    echoServer.start(4);
    loop.loop();
    
    return 0;
}
