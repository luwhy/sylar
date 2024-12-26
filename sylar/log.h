#ifndef SYLAR_LOG_H
#define SYLAR_LOG_H
#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include "stdarg.h"
#include "util.h"
#include <map>
#include "singleton.h"

#define SYLAR_LOG_LEVEL(logger, level)                                                                                        \
    if (logger->getLevel() <= level)                                                                                          \
    sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level, __FILE__, __LINE__, 0, sylar::GetPthreadId(), \
                                                                 sylar::GetFiberId(), time(0))))                              \
        .getSS()

#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::Level::DEBUG)
#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::Level::INFO)
#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::Level::WARN)
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::Level::FATAL)
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::Level::ERROR)

#define SYLAR_LOG_FMT_LEVEL(logger, level, fmt, ...)                                                           \
    if (logger->getLevel() <= level)                                                                           \
    sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level,                                \
                                                                 __FILE__, __LINE__, 0, sylar::GetPthreadId(), \
                                                                 sylar::GetFiberId(), time(0))))               \
        .getEvent()                                                                                            \
        ->format(fmt, __VA_ARGS__)

#define SYLAR_LOG_FMT_DEBUG(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::Level::DEBUG, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_INFO(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::Level::INFO, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_WARN(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::Level::WARN, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_ERROR(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::Level::ERROR, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_FATAL(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::Level::FATAL, fmt, __VA_ARGS__)

#define SYLAR_LOG_ROOT() sylar::LoggerMgr::GetInstance()->getRoot()

#define SYLAR_LOG_NAME(name) sylar::LoggerMgr::GetInstance()->getLogger(name)

namespace sylar
{
    class Logger;
    class LogManage;
    class LogLevel
    {
    public:
        enum class Level : int
        {
            UNKOWN = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5,

        };
        static const char *ToString(LogLevel::Level level);
        static LogLevel::Level FromString(const std::string &str);
    };

    // 日志事件
    class LogEvent
    {
    public:
        typedef std::shared_ptr<LogEvent> ptr;

        LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file, int32_t line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time);

        const char *getFile() const { return m_file; }

        int32_t getline() const { return m_line; }

        uint32_t getElapse() const { return m_elapse; }

        uint32_t getThreadId() const { return m_threadId; }

        uint32_t getFiberId() const { return m_fiberId; }

        uint64_t getTime() const { return m_time; }

        // const std::string &getContent() const { return m_content; }

        std::shared_ptr<Logger> getLogger() const
        {
            return m_logger;
        }

        std::string getContent() const { return m_ss.str(); }

        std::stringstream &getSS() { return m_ss; }

        LogLevel::Level getLevel() const { return m_level; }

        void format(const char *fmt, va_list al);

        void format(const char *fmt, ...);

    private:
        const char *m_file = nullptr; // 文件名
        int32_t m_line = 0;           // 行号
        uint32_t m_threadId = 0;      // 线程id
        uint32_t m_elapse = 0;        // 程序启动到现在的毫秒数
        uint32_t m_fiberId = 0;       // 协程id
        uint64_t m_time;              // 时间戳
        std::stringstream m_ss;
        std::string m_content;

        std::shared_ptr<Logger> m_logger;

        LogLevel::Level m_level;
    };

    class LogEventWrap
    {
    public:
        LogEventWrap(LogEvent::ptr e);
        ~LogEventWrap();
        std::stringstream &getSS();
        LogEvent::ptr getEvent();

    private:
        LogEvent::ptr m_event;
    };

    // 日志输出级别

    class FormatItem
    {
    public:
        typedef std::shared_ptr<FormatItem> ptr;

        FormatItem(const std::string &fmt = "");

        virtual ~FormatItem() {}

        virtual void format(std::ostream &os, std::shared_ptr<Logger> Logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };

    // 日志格式器
    class LogFormatter
    {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        LogFormatter(const std::string &pattern);

        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

        void init();
        bool isError() const
        {
            return this->m_error;
        }

    private:
        std::string m_pattern;
        std::vector<FormatItem::ptr> m_items;
        bool m_error = false;
    };

    // 日志输出地(可能有很多种，比如std::cout或者文件)
    class LogAppender
    {
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        virtual ~LogAppender() {}
        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        // virtual void log(LogLevel::Level level, LogEvent::ptr event) = 0;
        void setFormatter(LogFormatter::ptr val)
        {
            m_formatter = val;
        }
        LogFormatter::ptr getFormatter()
        {
            return m_formatter;
        }
        void setLevel(LogLevel::Level level)
        {
            this->m_level = level;
        }

    protected:
        LogLevel::Level m_level;
        LogFormatter::ptr m_formatter;
    };

    // 日志器
    class Logger : public std::enable_shared_from_this<Logger>
    {
        friend class LogManage;

    public:
        typedef std::shared_ptr<Logger> ptr;
        Logger(const std::string &name = "root");
        void log(LogLevel::Level level, LogEvent::ptr event);

        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);

        void addAppender(LogAppender::ptr appender);
        void delAppender(LogAppender::ptr appender);
        void clearAppenders();

        LogLevel::Level getLevel() const { return m_level; }

        void setLevel(LogLevel::Level val) { m_level = val; }

        std::string getName() const
        {
            return this->m_name;
        }
        void setFormatter(LogFormatter::ptr val);
        void setFormatter(const std::string &val);
        LogFormatter::ptr getFormatter();

    private:
        std::string m_name;                      // 日志名称
        LogLevel::Level m_level;                 // 日志级别
        std::list<LogAppender::ptr> m_appenders; // Appender集合
        LogFormatter::ptr m_formatter;
        Logger::ptr m_root;
    };

    // 输出到控制台的appender
    class StdoutLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event);
    };

    // 定义输出到文件的appender
    class FileLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string &filename);
        void log(Logger::ptr, LogLevel::Level level, LogEvent::ptr event);
        // 重新打开文件返回true
        bool reopen();

    private:
        std::string m_filename;
        std::ofstream m_filestream;
    };

    class LogManage
    {
    public:
        LogManage();
        Logger::ptr getLogger(const std::string &name);

        void init();

        Logger::ptr getRoot() const
        {
            return m_root;
        }

    private:
        std::map<std::string, Logger::ptr> m_logger;
        Logger::ptr m_root;
    };

    typedef sylar::Singleton<LogManage> LoggerMgr;

}

#endif