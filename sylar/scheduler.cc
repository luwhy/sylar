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
        MutexType lock(m_mutex);
        if (!m_stoping)
        {
            return;
        }
        m_stoping = false;
        SYLAR_ASSERT(m_threads.empty());
        m_threads.resize(m_threadCount);
        for (size_t i = 0; i < m_threadCount; ++i)
        {
            // run之后thread构造完成
            m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this), m_name + "_" + std::to_string(i)));
            m_threadIds.push_back(m_threads[i]->getId());
        }
        lock.unlock();
        if (m_rootFiber)
        {
            m_rootFiber->swapIn();
        }
    }

    void Scheduler::stop()
    {
        m_autoStop = true;
        if (m_rootFiber && m_threadCount == 0 && (m_rootFiber->m_state == Fiber::TERM || m_rootFiber->m_state == Fiber::INIT))
        {
            SYLAR_LOG_INFO(g_logger_s) << this << " stopped";
            m_stoping = true;
            if (stopping())
            {
                return;
            }
            // bool exit_on_this_fiber = false;

            // use caller
            if (m_rootThreadId != -1)
            {
                SYLAR_ASSERT(GetThis() == this);
            }
            else
            {
                SYLAR_ASSERT(GetThis() != this)
            }
            m_stoping = true;
            for (size_t i = 0; i < m_threadCount; ++i)
            {
                tickle();
            }
            if (m_rootFiber)
            {
                tickle();
            }
            if (stopping())
            {
                return;
            }
        }
    }

    void Scheduler::tickle()
    {
        SYLAR_LOG_INFO(g_logger_s) << "tickle";
    }
    void Scheduler::run()
    {
        setThis();
        if (sylar::GetPthreadId() != m_rootThreadId)
        {
            t_fiber = Fiber::GetThis().get();
        }
        Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
        Fiber::ptr cb_fiber;
        FiberAndThread ft;
        while (true)
        {
            ft.reset();
            bool tickle_me = false;
            {
                MutexType::Lock lock(m_mutex);
                auto it = m_fibers.begin();
                while (it != m_fibers.end())
                {
                    if (it->thread != -1 && it->thread != sylar::GetPthreadId())
                    {
                        ++it;
                        tickle_me = true;
                        continue;
                    }
                    SYLAR_ASSERT(it->fiber || it->cb)
                    if (it->fiber && it->fiber->m_state == Fiber::EXEC)
                    {
                        ++it;
                        continue;
                    }
                    ft = *it;
                    m_fibers.erase(it);
                }
            }
            if (tickle_me)
            {
                tickle();
            }
            // 唤醒执行
            if (ft.fiber && ((ft.fiber->m_state != Fiber::TERM) || ft.fiber->m_state != Fiber::EXCEPT))
            {
                ++m_activeThreadCount;
                ft.fiber->swapIn();
                --m_activeThreadCount;
                if (ft.fiber->m_state == Fiber::READY)
                {
                    schedule(ft.fiber);
                }
                else if (ft.fiber->m_state != Fiber::TERM && ft.fiber->m_state != Fiber::EXCEPT)
                {
                    ft.fiber->m_state = Fiber::HOLD;
                }

                ft.reset();
            }
            else if (ft.cb)
            {
                if (cb_fiber)
                {
                    cb_fiber->reset(ft.cb);
                }
                else
                {
                    cb_fiber.reset(new Fiber(ft.cb));
                }
                ft.reset();
                ++m_activeThreadCount;
                cb_fiber->swapIn();
                --m_activeThreadCount;
                if (cb_fiber->m_state == Fiber::READY)
                {
                    schedule(cb_fiber);
                    cb_fiber.reset();
                }
                // 直接释放掉
                else if (cb_fiber->m_state == Fiber::EXCEPT || cb_fiber->m_state == Fiber::TERM)
                {
                    cb_fiber->reset(nullptr);
                }
                else // if (cb_fiber->getState() != Fiber::TERM)
                {
                    cb_fiber->m_state = Fiber::HOLD;
                    cb_fiber.reset();
                }
            }
            else
            {
                if (idle_fiber->m_state == Fiber::TERM)
                {
                    SYLAR_LOG_INFO(g_logger_s) << "iddle fiber term";
                    break;
                }
                ++m_idleThreadCount;
                idle_fiber->swapIn();
                --m_idleThreadCount;
                if (idle_fiber->m_state != Fiber::TERM || idle_fiber->m_state != Fiber::EXCEPT)
                {
                    idle_fiber->m_state = Fiber::HOLD;
                }
            }
        }
    }
    bool Scheduler::stopping()
    {
        return m_autoStop && m_stoping && m_fibers.empty() && m_activeThreadCount == 0;
    }
    void Scheduler::setThis()
    {
        t_scheduler = this;
    }

    // 等待部分
    void Scheduler::idle()
    {
        SYLAR_LOG_INFO(g_logger_s) << "iddle";
    }
}