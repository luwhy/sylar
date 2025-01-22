#ifndef __SYLAR_UTIL__
#define __SYLAR_UTIL__
#include <sys/syscall.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <vector>
#include <string>
namespace sylar
{
    // 获取当前线程号
    pid_t GetPthreadId();
    // 获取当前携程号
    uint32_t GetFiberId();

    void BackTrace(std::vector<std::string> &bt, int size = 64, int skip = 2);

    std::string BacktraceToString(int size = 64, int skip = 2, const std::string &prefix = "");
}

#endif
