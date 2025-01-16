#ifndef __SYLAR_SCHEDULER_H
#define __SYLAR_SCHEDULER_H
#include <memory>
#include "fiber.h"
#include "thread.h"
#include <vector>
#include <list>
namespace sylar
{
    /**
     *
     *
     * 协程调度模块，主要是不同线程间协程调度操作
     * 线程池
     * 基类
     *
     */
    class Scheduler
    {
    public:
        // 智能指针类型
        typedef std::shared_ptr<Scheduler> ptr;

        typedef Mutex MutexType;
        /**
         *
         * use_caller 在哪个线程执行调度器，true，将此线程纳入调度器
         *
         */
        Scheduler(size_t threads = 1, bool use_caller = true, const std::string &name = "");
        virtual ~Scheduler();

        ~Scheduler();

        const std::string &getName() const { return m_name; }

        // 返回此类
        static Scheduler *GetThis();

        // 获取主协程
        static Fiber *GetMainFiber();

        void start();

        void stop();

        template <class FiberOrCb>
        void schedule(FiberOrCb fc, int thread = -1)
        {
            bool needTickle = false;
            {
                MutexType::Lock lock(m_mutex);
                needTickle = schedulerNoLock(fc, thread);
            }
            if (needTickle)
            {
                tickle();
            }
        }

        template <class InputeIterator>
        void schedule(InputeIterator begin, InputeIterator end)
        {
            bool need_tickle = false;
            {
                while (begin != end)
                {
                    need_tickle = schedulerNoLock(&(*begin)) || need_tickle;
                    begin++;
                }
            }
            if (need_tickle)
            {
                tickle();
            }
        }

    protected:
        virtual void tickle();
        void run();

    private:
        template <class FiberOrCb>
        void schedulerNoLock(FiberOrCb fc, int thread = -1)
        {
            bool need_tickle = m_fibers.empty();
            FiberAndThread ft(fc, thread);
            if (ft.fiber || ft.cb)
            {
                m_fibers.push_back(ft);
            }
            return need_tickle;
        }

    private:
        struct FiberAndThread
        {
        public:
            Fiber::ptr fiber;
            std::function<void()> cb;
            int thread;
            FiberAndThread(Fiber::ptr f, int thr) : fiber(f), thread(thr)
            {
            }
            FiberAndThread(Fiber::ptr *f, int thr) : thread(thr)
            {
                fiber.swap(*f);
            }
            FiberAndThread(std::function<void()> c, int thr) : cb(c), thread(thr)
            {
            }
            FiberAndThread(std::function<void()> *c, int thr) : thread(thr)
            {
                cb.swap(*c);
            }
            FiberAndThread() : thread(-1) {}
            void reset()
            {
                fiber = nullptr;
                cb = nullptr;
                thread = -1;
            }
        };

    private:
        MutexType m_mutex;

        // 线程池
        std::vector<Thread::ptr> m_threads;

        std::list<FiberAndThread> m_fibers;

        std::string m_name;

        Fiber::ptr m_rootFiber;

    protected:
        // 存放线程id
        std::vector<pid_t> m_threadIds;
        size_t m_threadCount = 0;
        size_t m_activeThreadCount = 0;
        size_t m_idleThreadCount = 0;
        bool m_stoping = true;
        bool m_autoStop = false;

        pid_t m_rootThreadId = 0;
    };
}
#endif