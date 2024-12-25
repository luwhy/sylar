# 操作笔记

## 约定优于配置

## 配置事件修改
当一个配置项发生修改的时候，可以反向通知对应的代码

## 日志系统整合
logs:
   name: root
   level: (debug, info,warn,error,fatal)


Logger::ptr g_log =SYLAR_LOG_NAME("system");
//m_root,m_system->m_root 当logger的appenders为空，使用root写logger