/*
 * Author: Montee
 * CreateData: 2025-1-30
 * UpdateData: 2024-1-30
 * Description: 配置管理器单例类的实现
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

// 日志配置结构体
struct LogConfig {
    std::string logLevel;      // 日志级别
    std::string logPath;       // 日志文件路径
    bool consoleOutput;        // 是否输出到控制台
    size_t maxFileSize;        // 单个日志文件最大大小（字节）
    size_t maxFiles;           // 最大日志文件数量
};

// 配置管理器的单例模式实现
class ConfigManager {
public:
    static ConfigManager* getInstance() {
        static ConfigManager instance;
        return &instance;
    }

    // 初始化配置管理器
    void init(const std::string& configPath);

    // 获取字符串类型的配置项
    std::string getStringValue(const std::string& key, const std::string& defaultValue = "") const;

    // 获取整数类型的配置项
    int getIntValue(const std::string& key, int defaultValue = 0) const;

    // 获取布尔类型的配置项
    bool getBoolValue(const std::string& key, bool defaultValue = false) const;

    // 获取日志配置
    LogConfig getLogConfig() const;

    // 获取指定 API 提供商的配置
    nlohmann::json getApiProviderConfig(const std::string& provider) const;

    // 获取所有 API 提供商的配置
    nlohmann::json getAllApiProviders() const;

    // 重新加载配置文件
    void reload();

private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    void loadConfig(const std::string& configPath);

    nlohmann::json config_;
    std::string configPath_;
};

// 配置管理器相关操作的宏封装
#define CONFIG_MANAGER ConfigManager::getInstance()

#endif // CONFIG_MANAGER_H