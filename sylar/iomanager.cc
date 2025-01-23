#include "iomanager.h"
#include <sys/epoll.h>
#include "unistd.h"
#include "log.h"
#include "fcntl.h"
namespace sylar
{
    static sylar::Logger::ptr g_logger_i = SYLAR_LOG_NAME("system");
    IOManager::IOManager(size_t threads, bool use_caller, const std::string &name) : Scheduler(threads, use_caller, name)
    {
        m_epfd = epoll_create(5000);

        int rt = pipe(m_tickleFds);

        SYLAR_ASSERT(rt);

        epoll_event event;

        // 给event 的前sizeof个字符，尽量是全0
        memset(&event, 0, sizeof(epoll_event));
        event.events = EPOLLIN || EPOLLET;
        event.data.fd = m_tickleFds[0];

        // 对文件描述符的控制，看第二个参数是获得或者复制等，并可能有第三个参数
        rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
        SYLAR_ASSERT(rt);

        rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
        SYLAR_ASSERT(rt);
        contextResize(32);
        start();
    }

    IOManager::~IOManager()
    {
        stop();
        close(m_epfd);
        close(m_tickleFds[0]);
        close(m_tickleFds[1]);
        for (size_t i = 0; i < m_fdContexts.size(); ++i)
        {
            if (m_fdContexts[i])
            {
                delete m_fdContexts[i];
                m_fdContexts[i] = nullptr;
            }
        }
    }

    int IOManager::addEvent(int fd, Event event, std::function<void()> cb)
    {
        FdContext *fd_ctx = nullptr;
        RWMutexType::ReadLock lock(m_rwmutex);
        if (m_fdContexts.size() > fd)
        {
            fd_ctx = m_fdContexts[fd];
            lock.unlock();
        }
        else
        {
            lock.unlock();
            RWMutex::WriteLock wrlock(m_rwmutex);
            contextResize(m_fdContexts.size() * 1.5);
            fd_ctx = m_fdContexts[fd];
        }
        FdContext::MutexType::Lock lock2(fd_ctx->mutex);
        if (fd_ctx->m_events && event)
        {
            SYLAR_LOG_ERROR(g_logger_i) << "event error";
        }
    }

    bool IOManager::delEvent(int fd, Event event)
    {
        return false;
    }

    bool IOManager::cancelEvent(int fd, Event event)
    {
        return false;
    }

    bool IOManager::cancelAll(int fd)
    {
        return false;
    }

    IOManager *IOManager::GetThis()
    {
        return nullptr;
    }

    void IOManager::tickle()
    {
    }
    bool IOManager::stopping()
    {
        return false;
    }
    void IOManager::idle()
    {
    }
    void IOManager::contextResize(size_t size)
    {
        m_fdContexts.resize(size);
        for (size_t i = 0; i < m_fdContexts.size(); ++i)
        {
            if (!m_fdContexts[i])
            {
                m_fdContexts[i] = new FdContext;
                m_fdContexts[i]->fd = i;
            }
        }
    }
}
