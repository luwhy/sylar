#ifndef __MACRO__H
#define __MACRO__H

#include <string.h>
#include <assert.h>
#include "log.h"

#define SYLAR_ASSERT(x)                                                                \
    if (!(x))                                                                          \
    {                                                                                  \
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION:" << #x << "\nbacktrace:\n"    \
                                          << sylar::BackTraceToString(100, 2, "    "); \
        assert(x);                                                                     \
    }

#define SYLAR_ASSERT2(x, w)                                                            \
    if (!(x))                                                                          \
    {                                                                                  \
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION:" << #x                        \
                                          << "\n"                                      \
                                          << w << "\nbacktrace:\n"                     \
                                          << sylar::BackTraceToString(100, 2, "    "); \
        assert(x);                                                                     \
    }

#endif