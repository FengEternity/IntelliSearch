/*
 * Author: Montee
 * CreateData: 2024-12-20
 * UpdateData: 2024-12-20
 * Description: 日志单例类的实现
 * Other: 该代码参考 https://blog.csdn.net/SaberJYang/article/details/128691465
 */

#ifndef LOGGRR_H
#define LOGGRR_H


#include <QObject>
#include <QString>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <filesystem>
#include "../config/ConfigManager.h"

// 使用ConfigManager.h中定义的LogConfig结构体

class Logger : public QObject
{
    Q_OBJECT
public:
    static Logger* getInstance()
    {
        static Logger instance;
        return &instance;
    }

    Q_INVOKABLE void log(const QString& level, const QString& message, const QString& component = QString());
    Q_INVOKABLE QString getLogLevel();
    Q_INVOKABLE void setLogLevel(const QString& level);

    // 添加QML日志处理方法
    Q_INVOKABLE void debug(const QString& message, const QString& component = QString()) { log("debug", message, component); }
    Q_INVOKABLE void info(const QString& message, const QString& component = QString()) { log("info", message, component); }
    Q_INVOKABLE void warn(const QString& message, const QString& component = QString()) { log("warn", message, component); }
    Q_INVOKABLE void error(const QString& message, const QString& component = QString()) { log("error", message, component); }

    std::shared_ptr<spdlog::logger> getLogger()
    {
        return loggerPtr;
    }

    void Init(const LogConfig& conf);

    std::string GetLogLevel();
    void SetLogLevel(const std::string& level);

signals:
    void logLevelChanged(const QString& newLevel);

private:
    Logger() = default;
    std::shared_ptr<spdlog::logger> loggerPtr;
};

// 定义宏用于QML日志
#define QMLLOG(level, message) Logger::getInstance()->log(level, message)
#define QMLDEBUG(message) Logger::getInstance()->debug(message)
#define QMLINFO(message) Logger::getInstance()->info(message)
#define QMLWARN(message) Logger::getInstance()->warn(message)
#define QMLERROR(message) Logger::getInstance()->error(message)

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