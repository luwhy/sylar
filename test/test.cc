#include<iostream>
#include"../sylar/log.h"
#include"../sylar/util.h"
int main()
{
    std::cout<<"start"<<std::endl;
    sylar::Logger::ptr logger(new sylar::Logger);
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));
    sylar::LogEvent::ptr event(new sylar::LogEvent(logger,__FILE__,__LINE__,0,sylar::GetPthreadId(),2,time(0)));
    logger->log(sylar::LogLevel::Level::DEBUG,event);
    return 0;
}