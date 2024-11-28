#include"util.h"

namespace sylar{
    pid_t GetPthreadId(){
        return syscall(SYS_getpid);
    }
}