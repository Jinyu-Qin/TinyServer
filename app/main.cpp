#include <iostream>
#include <string>
#include <glog/logging.h>

class GoogleLoggingInitializer {
public:
    GoogleLoggingInitializer() {
        // FIXME 这里改成动态获取程序名
        google::InitGoogleLogging("tinyserver");
        FLAGS_log_dir = "./log";
    }
};

GoogleLoggingInitializer glogInit;

int main(int argc, char * argv[]) {
    
    return 0;
}
