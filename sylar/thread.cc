#include "thread.h"
#include "log.h"
#include "util.h"
#include <atomic>
namespace sylar
{
    /**
     * 有且只有 thread_local 关键字修饰的变量具有线程（thread）周期，
     * 这些变量在线程开始的时候被生成，在线程结束的时候被销毁，
     * 并且每一个线程都拥有一个独立的变量实例。
     *
     */
    static thread_local Thread *t_thread = nullptr;
    static thread_local std::string t_thread_name = "UNKOWN";
    static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

    /**
     * 静态方法
     */
    Thread *
    Thread::GetThis()
    {
        return t_thread;
    }

    /**
     * 静态方法
     */
    const std::string &Thread::GetName()
    {
        return t_thread_name;
        // TODO: 在此处插入 return 语句
    }

    void Thread::SetName(const std::string &name)
    {
        if (name.empty())
        {
            return;
        }
        if (t_thread)
        {
            t_thread->m_name = name;
        }
        t_thread_name = name;
    }

    // run是静态函数，每个线程都有一个静态的t_thread,让t_thread调用新的Thread对象
    void *Thread::run(void *arg)
    {
        Thread *thread = (Thread *)arg;
        t_thread = thread;
        t_thread_name = thread->m_name;
        thread->m_id = sylar::GetPthreadId();
        // 设置线程名称，顶多16个字符数
        pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());
        std::function<void()> cb;

        cb.swap(thread->m_cb);
        thread->m_semaphore.notify();
        cb();
        return 0;
    }

    // 构造函数
    Thread::Thread(std::function<void()> cb, const std::string &name) : m_cb(cb), m_name(name)
    {
        if (name.empty())
        {
            m_name = "UNKOWN";
        }
        // 把本对象作为参数传递给线程
        int rt = pthread_create(&m_thread, NULL, &Thread::run, this);
        if (rt)
        {
            SYLAR_LOG_ERROR(g_logger) << "pthread_create thread fail,rt= " << rt << " name= " << name << std::endl;
            throw std::logic_error("pthread_create error");
        }
        m_semaphore.wait();
    }

    // 析构
    Thread::~Thread()
    {
        if (m_thread)
        {
            pthread_detach(m_thread);
        }
    }

    // 回收线程资源
    void Thread::join()
    {
        if (m_thread)
        {
            int rt = pthread_join(m_thread, NULL);
            if (rt)
            {
                SYLAR_LOG_ERROR(g_logger) << "pthread_join thread fail,rt= " << rt << " name= " << m_name << std::endl;
                throw std::logic_error("pthread_join error");
            }
            m_thread = 0;
        }
    }

    Semaphore::Semaphore(uint32_t count)
    {
        if (sem_init(&m_semaphore, 0, count))
        {
            throw std::logic_error("sem_init error");
        }
    }

    Semaphore::~Semaphore()
    {
        sem_destroy(&m_semaphore);
    }
    void Semaphore::wait()
    {
        if (sem_wait(&m_semaphore))
        {
            return;
        }
    }
    void Semaphore::notify()
    {
        if (sem_post(&m_semaphore))
        {
            throw std::logic_error("sem_post error");
        }
    }

    /**
     *
     * 自旋锁
     *
     */
    SpinLock::SpinLock()
    {
        pthread_spin_init(&m_lock, 0);
    }
    SpinLock::~SpinLock()
    {
        pthread_spin_destroy(&m_lock);
    }
    void SpinLock::lock()
    {
        pthread_spin_lock(&m_lock);
    }
    void SpinLock::unlock()
    {
        pthread_spin_unlock(&m_lock);
    }

    /***
     *
     * CAS机制
     *
     */
    CASLock::CASLock()
    {
        this->m_mutex.clear();
    }
    CASLock::~CASLock()
    {
    }
    void CASLock::lock()
    {
        while (std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire))
            ;
    }
    void CASLock::unlokck()
    {
        std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
    }

    /**
     *
     * 空锁
     *
     */
    NullMutex::NullMutex()
    {
    }
    NullMutex::~NullMutex()
    {
    }

    void NullMutex::lock()
    {
    }
    void NullMutex::unlock()
    {
    }
}