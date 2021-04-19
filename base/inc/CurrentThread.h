#ifndef __CURRENTTHREAD_H__
#define __CURRENTTHREAD_H__

#include <unistd.h>

namespace CurrentThread {

pid_t tid();
const char * tidString();
int tidStringLength();
const char * name();
void setName(const char * name);
bool isMainThread();

}

#endif //__CURRENTTHREAD_H__
