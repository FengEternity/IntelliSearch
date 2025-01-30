/*
 * Author: Montee
 * CreateData: 2024-12-20
 * UpdateData: 2024-12-20
 * Description: 日志单例类的实现
 * Other: 该代码参考 https://blog.csdn.net/SaberJYang/article/details/128691465
 */

#ifndef LOGGER_H
#define LOGGER_H

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include "../config/ConfigManager.h"

// 日志的单例模式
class Logger
{
public:
    static Logger* getInstance()
    {
        static Logger instance;
        return &instance;
    }

    //c++14返回值可设置为auto
    std::shared_ptr<spdlog::logger> getLogger()
    {
        return loggerPtr;
    }

    void Init(const LogConfig& conf);

    std::string GetLogLevel();

    void SetLogLevel(const std::string& level);

private:
    Logger() = default;
    std::shared_ptr<spdlog::logger> loggerPtr;
};

// 日志相关操作的宏封装
#define INITLOG(conf)      Logger::getInstance()->Init(conf)
#define GETLOGLEVEL()      Logger::getInstance()->GetLogLevel()
#define SETLOGLEVEL(level) Logger::getInstance()->SetLogLevel(level)
#define BASELOG(logger, level, ...) (logger)->log(spdlog::source_loc{__FILE__, __LINE__, __func__}, level, __VA_ARGS__)
#define TRACELOG(...)     BASELOG(Logger::getInstance()->getLogger(), spdlog::level::trace, __VA_ARGS__)
#define DEBUGLOG(...)     BASELOG(Logger::getInstance()->getLogger(), spdlog::level::debug, __VA_ARGS__)
#define INFOLOG(...)      BASELOG(Logger::getInstance()->getLogger(), spdlog::level::info, __VA_ARGS__)
#define WARNLOG(...)      BASELOG(Logger::getInstance()->getLogger(), spdlog::level::warn, __VA_ARGS__)
#define ERRORLOG(...)     BASELOG(Logger::getInstance()->getLogger(), spdlog::level::err, __VA_ARGS__)
#define CRITICALLOG(...)  BASELOG(Logger::getInstance()->getLogger(), spdlog::level::critical, __VA_ARGS__)

#endif // LOGGER_H