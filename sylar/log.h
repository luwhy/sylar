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
namespace sylar
{
    class Logger;
    // 日志事件
    class LogEvent
    {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent(const char*file,int32_t line,uint32_t elapse,uint32_t thread_id,uint32_t fiber_id,uint64_t time);

        const char *getFile() const { return m_file; }

        int32_t getline() const { return m_line; }

        uint32_t getElapse() const { return m_elapse; }

        uint32_t getThreadId() const { return m_threadId; }

        uint32_t getFiberId() const { return m_fiberId; }

        uint64_t getTime() const { return m_time; }

        //const std::string &getContent() const { return m_content; }

        std::shared_ptr<Logger> getLogger() const { return m_logger; }

        std::string getContent() const {return m_ss.str();
        }
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
    };

    //日志输出级别
    class LogLevel
    {
    public:
        enum class Level : int
        {
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };
        static const char *ToString(LogLevel::Level level);
    };

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

    protected:
        LogLevel::Level m_level;
        LogFormatter::ptr m_formatter;
    };

    // 日志器
    class Logger : public std::enable_shared_from_this<Logger>
    {
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

        LogLevel::Level getLevel() const { return m_level; }

        void setLevel(LogLevel::Level val) { m_level = val; }

        const std::string &getName() { return m_name; }

    private:
        std::string m_name;                      // 日志名称
        LogLevel::Level m_level;                 // 日志级别
        std::list<LogAppender::ptr> m_appenders; // Appender集合
        LogFormatter::ptr m_formatter;
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
}

#endif