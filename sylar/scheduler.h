#ifndef __SYLAR_SCHEDULER_H
#define __SYLAR_SCHEDULER_H
#include <memory>
#include "fiber.h"

class Scheduler
{
public:
    typedef std::shared_ptr<Scheduler> ptr;

private:
};
#endif