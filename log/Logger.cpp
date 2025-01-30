//
// Created by montee on 24-12-20.
//

#include "Logger.h"
#include "../config/ConfigManager.h"
#include <iostream>

void Logger::Init(const LogConfig& conf)
{
    try {
        // 确保日志目录存在
        std::filesystem::path logPath(conf.logPath);
        std::filesystem::create_directories(logPath.parent_path());
        
        //自定义的sink
        loggerPtr = spdlog::rotating_logger_mt("base_logger", conf.logPath.c_str(), conf.maxFileSize, conf.maxFiles);
        //设置格式
        //参见文档 https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
        //[%Y-%m-%d %H:%M:%S.%e] 时间
        //[%l] 日志级别
        //[%t] 线程
        //[%s] 文件
        //[%#] 行号
        //[%!] 函数
        //[%v] 实际文本
        loggerPtr->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] [%s %!:%#] %v");

        // 设置日志级别
        loggerPtr->set_level(spdlog::level::from_str(conf.logLevel));
        // 设置刷新日志的日志级别，当出现level或更高级别日志时，立刻刷新日志到  disk
        loggerPtr->flush_on(spdlog::level::from_str(conf.logLevel));
    } catch (const std::exception& e) {
        // 记录初始化过程中的错误
        std::cerr << "Logger initialization failed: " << e.what() << std::endl;
    }
}

/*
 * trace 0
 * debug 1
 * info 2
 * warn 3
 * error 4
 * critical 5
 * off 6 (not use)
 */
std::string Logger::GetLogLevel()
{
    try {
        auto level = loggerPtr->level();
        return spdlog::level::to_string_view(level).data();
    } catch (const std::exception& e) {
        std::cerr << "Failed to get log level: " << e.what() << std::endl;
        return "unknown";
    }
}

void Logger::SetLogLevel(const std::string& log_level)
{
    try {
        auto level = spdlog::level::from_str(log_level);
        if (level == spdlog::level::off)
        {
            WARNLOG("Given invalid log level {}", log_level);
        }
        else
        {
            loggerPtr->set_level(level);
            loggerPtr->flush_on(level);
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to set log level: " << e.what() << std::endl;
    }
}

// 调用示例

// #include "logger.h"
//
// int main()
// {
//     // 定义日志配置项
//     LogConfig conf = {
//         .level = "trace",
//         .path  = "logger_test.log",
//         .size  = 5 * 1024 * 1024,
//         .count = 10,
//     };
//     INITLOG(conf);
//     // 日志初始级别为trace
//     TRACELOG("current log level is {}", GETLOGLEVEL());
//     TRACELOG("this is trace log");
//     DEBUGLOG("this is debug log");
//     INFOLOG("this is info log");
//     WARNLOG("this is warning log");
//     ERRORLOG("this is a error log");
//     CRITICALLOG("this is critical log");
//
//     // 改为warning级别后，trace、debug、info级别日志不会输出了
//     SETLOGLEVEL("warn");
//     WARNLOG("after set log level to warning");
//     TRACELOG("this is trace log");
//     DEBUGLOG("this is debug log");
//     INFOLOG("this is info log");
//     WARNLOG("this is warning log");
//     ERRORLOG("this is a error log");
//     CRITICALLOG("this is critical log");
//
//     return 0;
// }
