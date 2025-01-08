#include "thread.h"

namespace sylar
{
    /**
     * 静态方法
     */
    Thread *Thread::GetThis()
    {
        return nullptr;
    }
    /**
     * 静态方法
     */
    const std::string &Thread::GetName()
    {
        // TODO: 在此处插入 return 语句
    }
    Thread::Thread(std::function<void()> cb, const std::string &name)
    {
    }

    Thread::~Thread()
    {
    }

    void Thread::join()
    {
    }

}