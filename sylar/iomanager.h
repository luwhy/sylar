#ifndef __SYLAR__IOMANAGE_H__
#define __SYLAR__IOMANAGE_H__
#include "scheduler.h"
#include "fiber.h"
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
            int fd;                // 事件关联句柄
            MutexType mutex;
        };

        RWMutexType rwmutex;

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

    private:
        int m_epfd = 0;
        };

}
#endif