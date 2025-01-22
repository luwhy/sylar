#include "../sylar/sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test()
{
    SYLAR_LOG_INFO(g_logger) << "sylar_log_info";
}
int main()
{

    sylar::Scheduler sc;
    sc.start();
    sc.schedule(&test);
    sc.stop();
    return 0;
}
