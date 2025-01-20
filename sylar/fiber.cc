#include "fiber.h"
#include <atomic>
#include "config.h"
#include "macro.h"
#include "log.h"
namespace sylar
{
    sylar::Logger::ptr g_logger_f = SYLAR_LOG_NAME("system");
    static std::atomic<uint64_t> s_fiber_id{0};

    static std::atomic<uint64_t> s_fiber_count{0};

    // 每个线程拥有其自身的对象实例,这个指代当前协程
    static thread_local Fiber *t_fiber = nullptr;

    // 这个是主协程
    static thread_local Fiber::ptr t_threadFiber = nullptr;

    // 栈大小
    static ConfigVar<uint32_t>::ptr g_fiber_stack_size = Config::Lookup<uint32_t>("fiber.stack_size", 1024 * 1024, "fiber stack size");
    // 分配栈
    class MallocStackAllocator
    {

    public:
        static void *Alloc(size_t size)
        {
            return malloc(size);
        }
        static void Dealloc(void *vp, size_t size)
        {
            return free(vp);
        }
    };

    using StackAllocator = MallocStackAllocator;

    Fiber::Fiber()
    {
        m_state = State::EXEC;
        SetThis(this);
        if (getcontext(&m_ctx))
        {
            SYLAR_ASSERT2(false, "getcontext")
        }
        ++s_fiber_count;
    }
    Fiber::Fiber(std::function<void()> cb, size_t stacksize) : m_cb(cb)
    {
        m_id = s_fiber_id;
        ++s_fiber_id;
        ++s_fiber_count;
        m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();
        // SYLAR_LOG_DEBUG(g_logger_f) << "Fiber::Fiber stacksize " << m_stacksize;
        //  初始化结构体，病将当前上下文信息保存在m_ctx中
        // 分配栈空间
        m_stack = StackAllocator::Alloc(m_stacksize);
        if (getcontext(&m_ctx))
        {
            SYLAR_ASSERT(false)
        }
        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;
        // 创建啊一个新的上下文，开始执行MainFunc
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
        SYLAR_LOG_DEBUG(g_logger_f) << "Fiber::Fiber id= " << m_id;
    }
    Fiber::~Fiber()
    {
        --s_fiber_count;
        if (m_stack)
        {
            SYLAR_ASSERT(m_state == State::TERM || m_state == State::INIT || m_state == State::EXCEPT)
            StackAllocator::Dealloc(m_stack, m_stacksize);
        }
        else
        {
            SYLAR_ASSERT(!m_cb)
            SYLAR_ASSERT(m_state == State::EXEC)
            Fiber *cur = t_fiber;
            if (cur == this)
            {
                SetThis(nullptr);
            }
        }
        SYLAR_LOG_DEBUG(g_logger_f) << "Fiber::~Fiber id= " << m_id;
    }

    // 重置协程状态，即恢复至主协程
    void Fiber::reset(std::function<void()> cb)
    {
        SYLAR_ASSERT(m_stack)
        SYLAR_ASSERT(m_state == State::TERM || m_state == State::INIT || m_state == State::EXCEPT);
        m_cb = cb;
        // 返回值-1时，是执行失败，将当前执行的上下文保存在CPU中
        if (getcontext(&m_ctx))
        {
            SYLAR_ASSERT(false)
        }
        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;
        // 初始化一个ucontext_t, func参数指明了该context的入口函数
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
        m_state = State::INIT;
    }

    // 切换到当前协程
    void Fiber::swapIn()
    {
        SetThis(this);
        SYLAR_ASSERT(m_state != State::EXEC);
        if (swapcontext(&(t_threadFiber->m_ctx), &m_ctx))
        {
            SYLAR_ASSERT2(false, "Fiber")
        }
    }
    // 把当前协程切换到后台，把main协程调出来
    void Fiber::swapOut()
    {
        SetThis((t_threadFiber).get());
        if (swapcontext(&m_ctx, &(t_threadFiber->m_ctx)))
        {
            SYLAR_ASSERT2(false, "Fiber")
        }
    }
    Fiber::State Fiber::getState()
    {
        return m_state;
    }
    void Fiber::setState(Fiber::State state)
    {
        this->m_state = state;
    }
    void Fiber::SetThis(Fiber *f)
    {
        t_fiber = f;
    }
    /**
     *
     * 如果还没协程，本协程设置为main协程
     *
     */
    Fiber::ptr Fiber::GetThis()
    {
        if (t_fiber)
        {
            return t_fiber->shared_from_this();
        }
        Fiber::ptr main_fiber(new Fiber);
        SYLAR_ASSERT(t_fiber == main_fiber.get());
        t_threadFiber = main_fiber;
        return t_fiber->shared_from_this();
    }
    // 协程切换到后台，并且设置为ready状态
    void Fiber::YieldToReady()
    {
        Fiber::ptr cur = GetThis();
        cur->m_state = State::READY;
        cur->swapOut();
    }
    // 协程切换到后台，并且设置为hold状态
    void Fiber::YieldToHold()
    {
        Fiber::ptr cur = GetThis();
        cur->m_state = State::HOLD;
        cur->swapOut();
    }
    uint64_t Fiber::TotalFibers()
    {
        return s_fiber_count;
    }

    /**
     *
     * static 函数
     *
     */
    void Fiber::MainFunc()
    {
        Fiber::ptr cur = GetThis();
        SYLAR_ASSERT(cur);
        try
        {
            cur->m_cb();
            cur->m_cb = nullptr;
            cur->m_state = State::TERM;
        }
        catch (std::exception &e)
        {
            cur->m_state = State::EXCEPT;
            SYLAR_LOG_ERROR(g_logger_f) << "Fiber Except: " << e.what();
        }
        catch (...)
        {
            cur->m_state = State::EXCEPT;
            SYLAR_LOG_ERROR(g_logger_f) << "Fiber Except";
        }
        auto raw_ptr = cur.get();
        cur.reset();
        raw_ptr->swapOut();
        SYLAR_ASSERT2(false, "never reach");
    }
    uint64_t Fiber::GetFiberId()
    {
        if (t_fiber)
        {
            return t_fiber->getId();
        }
        else
        {
            return 0;
        }
    }
}