#include<sys/syscall.h>
#include<pthread.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdint.h>
namespace sylar{
    //获取当前线程号
    pid_t GetPthreadId();
    //获取当前携程号
    uint32_t GetFiberId();
}
