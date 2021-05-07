#include <iostream>
#include <string>
#include <glog/logging.h>

static void initGlog(const char * name) {
    google::InitGoogleLogging(name);
    FLAGS_log_dir = "./log";
}

int main(int argc, char * argv[]) {
    initGlog(argv[0]);
    
    return 0;
}
