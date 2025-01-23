#ifndef __SYLAR__IOMANAGE_H__
#define __SYLAR__IOMANAGE_H__
#include "scheduler.h"
#include "fiber.h"
#include <atomic>
#include <vector>
#include "macro.h"
namespace sylar
{

    class IOManager : public Scheduler
    {
    public:
        typedef std::shared_ptr<IOManager> ptr;

        typedef RWMutex RWMutexType;

        enum Event
        {
            NONE = 0x0,
            READ = 0x1,
            WRITE = 0x2
        };

    private:
        struct FdContext
        {
            typedef Mutex MutexType;
            struct EventContext
            {
                Scheduler *scheduler = nullptr; // 事件执行的scheduler
                Fiber::ptr fiber;               // 事件协程
                std::function<void()> cb;       // 事件的回调函数
            };
            EventContext read;     // 读事件
            EventContext write;    // 写事件
            Event m_events = NONE; // 已经注册的事件
            int fd = 0;            // 事件关联句柄
            MutexType mutex;
        };

    public:
        IOManager(size_t threads = 1, bool use_caller = true, const std::string &name = "");
        virtual ~IOManager();

        // 1 success,0 retry,-1 error
        int addEvent(int fd, Event event, std::function<void()> cb = nullptr);

        bool delEvent(int fd, Event event);

        bool cancelEvent(int fd, Event event);

        bool cancelAll(int fd);

        static IOManager *GetThis();

    protected:
        virtual void tickle() override;

        virtual bool stopping() override;

        virtual void idle() override;

        void contextResize(size_t size);

    private:
        int m_epfd = 0;

        // 传给pipe的参数
        int m_tickleFds[2];

        // 等待执行的事件数量
        std::atomic<size_t> m_penddingEventCount = {0};

        RWMutexType m_rwmutex;

        std::vector<FdContext *> m_fdContexts;

        // epoll_create epoll_wait epoll ctl
    };

}
#endif