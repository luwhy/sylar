#include "util.h"
#include <execinfo.h>
#include "log.h"
namespace sylar
{
    sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");
    pid_t GetPthreadId()
    {
        /**
         * 2种获取线程id的方法
         * 但是pthread_self获取的结果只在本进程内使用
         * syscall(SYS_getpid)是相对内核的线程id，不同进程内的线程之间也是不同的
         * pid_t t=pthread_self();
         */

        return syscall(SYS_getpid);
    }

    uint32_t GetFiberId()
    {
        return 0;
    }
    void BackTrace(std::vector<std::string> &bt, int size, int skip)
    {
        void **array = (void **)malloc((sizeof(void *) * size));

        // 双冒号前不加东西表示外部全局变量
        size_t s = ::backtrace(array, size);

        char **strings = backtrace_symbols(array, s);
        if (strings == NULL)
        {
            SYLAR_LOG_ERROR(g_logger) << "backtrace_synbols error";
            free(array);
            return;
        }
        for (size_t i = skip; i < s; ++i)
        {
            bt.push_back(strings[i]);
        }
        free(strings);
        free(array);
    }
    std::string BackTraceToString(int size, int skip, const std::string &prefix = "")
    {
        std::vector<std::string> bt;
        BackTrace(bt, size, skip);
        std::stringstream ss;
        for (size_t i = 0; i < bt.size(); ++i)
        {
            ss << prefix << bt[i] << std::endl;
        }
        return ss.str();
    }
}