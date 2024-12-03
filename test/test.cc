#include<iostream>
#include"../sylar/log.h"
#include"../sylar/util.h"
int main()
{
    sylar::Logger::ptr logger(new sylar::Logger);
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));
    //sylar::LogEvent::ptr event(new sylar::LogEvent(logger,sylar::LogLevel::Level::DEBUG,__LINE__,0,sylar::GetPthreadId(),2,time(0)));
    //logger->log(sylar::LogLevel::Level::DEBUG,event);
    sylar::FileLogAppender::ptr file_appender(new sylar::FileLogAppender("./log.txt"));
    file_appender->setLevel(sylar::LogLevel::Level::ERROR);
    logger->addAppender(file_appender);


    sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d%T%m%n"));
    file_appender->setFormatter(fmt);
    std::cout<<"hello log"<<std::endl;
    SYLAR_LOG_INFO(logger)<<"test marco";
    SYLAR_LOG_FMT_ERROR(logger, "test macro fmt error %s", "aa");

    auto l=sylar::LoggerMgr::GetInstance()->getLogger("xx");
    SYLAR_LOG_INFO(l)<<"xxx";
    return 0;
}