#include "../sylar/sylar.h"
#include <assert.h>

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_assert()
{
    SYLAR_LOG_INFO(g_logger) << sylar::BackTraceToString(10, 0, "");
    SYLAR_ASSERT(false);
}
int main()
{
    test_assert();
    return 0;
}