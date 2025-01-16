#include "../sylar/sylar.h"
sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run_in_fiber()
{
    SYLAR_LOG_INFO(g_logger) << "run in fiber begin";
    sylar::Fiber::YieldToHold();
    SYLAR_LOG_INFO(g_logger) << "run in fiber end";
    sylar::Fiber::YieldToHold();
}
void testFiber()
{
    SYLAR_LOG_INFO(g_logger) << "main beigin 1";
    {
        sylar::Fiber::GetThis();
        SYLAR_LOG_INFO(g_logger) << "main beigin";
        sylar::Fiber::ptr fiber(new sylar::Fiber(run_in_fiber, 0));
        fiber->swapIn();
        SYLAR_LOG_INFO(g_logger) << "main after swapIn";
        fiber->swapIn();
        SYLAR_LOG_INFO(g_logger) << "main end";
        fiber->swapIn();
    }
    SYLAR_LOG_INFO(g_logger) << "main end 2";
}

int main()
{
    sylar::Thread::SetName("main");
    std::vector<sylar::Thread::ptr> ptrs;
    for (int i = 0; i < 3; ++i)
    {
        ptrs.push_back(sylar::Thread::ptr(new ::sylar::Thread(&testFiber, "name" + std::to_string(i))));
    }
    for (int i = 0; i < 3; ++i)
    {
        ptrs[i]->join();
    }
}