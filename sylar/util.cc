#include"util.h"

namespace sylar{
    pid_t GetPthreadId(){
        //2种获取线程id的方法
        //但是pthread_self获取的结果只在本进程内使用
        //syscall(SYS_getpid)是相对内核的线程id，不同进程内的线程之间也是不同的
        //pid_t t=pthread_self();

        return syscall(SYS_getpid);

    }

    uint32_t GetFiberId(){
        return 0;
    }
}