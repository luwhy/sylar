#ifndef __SYLAR_FIBER_H__
#define __SYLAR_FIBER_H__

#include <memory>
#include <functional>
#include <ucontext.h>
#include "thread.h"

namespace sylar
{
    /**
     *
     *
     * 把自己当成只能指针
     * 可以获取自身智能指针
     *
     */
    class Fiber : public std::enable_shared_from_this<Fiber>
    {
    public:
        typedef std::shared_ptr<Fiber> ptr;
        enum State
        {
            INIT,
            HOLD,
            EXEC,
            TERM,
            READY,
            EXCEPT
        };

    private:
        // 不允许创建默认构造函数
        Fiber();

    public:
        Fiber(std::function<void()> cb, size_t stacksize = 0);

        ~Fiber();
        // 充值协程函数，并重置状态
        void reset(std::function<void()> cb);
        // 切换到当前协程执行
        void swapIn();
        // 切换到后台执行
        void swapOut();

        uint64_t getId() const { return m_id; }

        Fiber::State getState();

        void setState(Fiber::State state);

    public:
        // 设置当前协程
        static void SetThis(Fiber *f);
        // 返回当前协程
        static Fiber::ptr GetThis();
        // 协程切换到后台，并且设置为Ready状态
        static void YieldToReady();
        // 协程切换到后台，并设置为hold状态
        static void YieldToHold();
        // 总协程数
        static uint64_t TotalFibers();

        static void MainFunc();

        static uint64_t GetFiberId();

    private:
        uint64_t m_id = 0;
        uint32_t m_stacksize = 0;
        State m_state = State::INIT;
        // 提供所谓的用户上下文信息
        ucontext_t m_ctx;

        void *m_stack = nullptr;
        std::function<void()> m_cb;
    };

}

#endif