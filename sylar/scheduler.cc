#include "scheduler.h"
#include "log.h"
#include "macro.h"
namespace sylar
{
    static sylar::Logger::ptr g_logger_s = SYLAR_LOG_NAME("system");

    // 协程调度器指针
    static thread_local Scheduler *t_scheduler = nullptr;

    //  主协程
    static thread_local Fiber *t_fiber = nullptr;

    Scheduler::Scheduler(size_t threads, bool use_caller, const std::string &name) : m_name(name)
    {
        SYLAR_ASSERT(threads > 0);
        /**
         *
         * 放入线程池的话，作为主协程
         * 不放入线程池的话，专职协程调度
         *
         */
        if (use_caller)
        {
            sylar::Fiber::GetThis();
            // 可用线程-1
            --threads;
            // 协程调度器不允许多个协程进行调度
            SYLAR_ASSERT(GetThis() == nullptr);
            t_scheduler = this;

            // 新建作为主协程的流程
            m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this)));
            sylar::Thread::SetName(m_name);

            // 设置主协程
            t_fiber = m_rootFiber.get();
            m_rootThreadId = sylar::GetPthreadId();
            m_threadIds.push_back(m_rootThreadId);
        }
        else
        {
            m_rootThreadId = -1;
        }
        m_threadCount = threads;
    }

    Scheduler::~Scheduler()
    {
        SYLAR_ASSERT(m_stoping);
        if (GetThis() == this)
        {
            t_scheduler = nullptr;
        }
    }

    Scheduler *Scheduler::GetThis()
    {
        return t_scheduler;
    }

    Fiber *Scheduler::GetMainFiber()
    {
        return t_fiber;
    }

    void Scheduler::start()
    {
    }

    void Scheduler::stop()
    {
    }

    void Scheduler::tickle()
    {
    }
    void Scheduler::run()
    {
    }
}