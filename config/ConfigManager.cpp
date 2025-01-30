#include "ConfigManager.h"
#include "log/Logger.h"
#include <fstream>
#include <iostream>
#include <filesystem>

void ConfigManager::init(const std::string& configPath) {
    configPath_ = configPath;
    loadConfig(configPath);
}

void ConfigManager::loadConfig(const std::string& configPath) {
    try {
        std::vector<std::filesystem::path> searchPaths;
        std::filesystem::path currentPath = std::filesystem::current_path();
        std::filesystem::path exePath = currentPath;
        
        // 添加搜索路径
        searchPaths.push_back(configPath);  // 直接路径
        searchPaths.push_back(currentPath / configPath);  // 当前目录
        
        // 向上查找，直到找到项目根目录（最多查找5层）
        std::filesystem::path tempPath = currentPath;
        for (int i = 0; i < 5 && tempPath.has_parent_path(); ++i) {
            searchPaths.push_back(tempPath / configPath);
            searchPaths.push_back(tempPath / "config" / "config.json");  // 项目标准位置
            tempPath = tempPath.parent_path();
        }
        
        // 尝试所有可能的路径
        std::string triedPaths;
        for (const auto& path : searchPaths) {
            if (std::filesystem::exists(path)) {
                std::ifstream configFile(path);
                if (configFile.is_open()) {
                    configFile >> config_;
                    configPath_ = path.string();  // 更新实际使用的配置文件路径
                    return;
                }
            }
            triedPaths += "\n              " + path.string();
        }
        
        // 如果所有路径都失败，抛出异常
        throw std::runtime_error("无法打开配置文件: " + configPath + "\n尝试的路径: " + triedPaths);
    } catch (const std::exception& e) {
        throw std::runtime_error("加载配置文件失败: " + std::string(e.what()));
    }
}

std::string ConfigManager::getStringValue(const std::string& key, const std::string& defaultValue) const {
    try {
        if (config_.contains(key)) {
            return config_[key].get<std::string>();
        }
    } catch (const std::exception& e) {
        WARNLOG("获取配置项 {} 失败: {}", key, e.what());
    }
    return defaultValue;
}

int ConfigManager::getIntValue(const std::string& key, int defaultValue) const {
    try {
        if (config_.contains(key)) {
            return config_[key].get<int>();
        }
    } catch (const std::exception& e) {
        WARNLOG("获取配置项 {} 失败: {}", key, e.what());
    }
    return defaultValue;
}

bool ConfigManager::getBoolValue(const std::string& key, bool defaultValue) const {
    try {
        if (config_.contains(key)) {
            return config_[key].get<bool>();
        }
    } catch (const std::exception& e) {
        WARNLOG("获取配置项 {} 失败: {}", key, e.what());
    }
    return defaultValue;
}

LogConfig ConfigManager::getLogConfig() const {
    LogConfig logConfig;
    try {
        // 使用绝对路径来存储日志
        std::filesystem::path basePath = std::filesystem::current_path();
        std::filesystem::path defaultLogPath = basePath / "logs/app.log";
        
        if (config_.contains("log")) {
            const auto& logJson = config_["log"];
            logConfig.logLevel = logJson.value("level", "info");
            std::filesystem::path logPath = basePath / logJson.value("path", "logs/app.log");
            logConfig.logPath = logPath.string();
            logConfig.maxFileSize = logJson.value("size", 1024 * 1024); // 默认 1MB
            logConfig.maxFiles = logJson.value("count", 3);
        } else {
            // 如果没有日志配置，使用默认值
            logConfig.logLevel = "info";
            logConfig.logPath = defaultLogPath.string();
            logConfig.maxFileSize = 1024 * 1024; // 默认 1MB
            logConfig.maxFiles = 3;
        }
    } catch (const std::exception& e) {
        // 如果出现异常，使用默认配置
        std::filesystem::path basePath = std::filesystem::current_path();
        std::filesystem::path defaultLogPath = basePath / "logs/app.log";
        logConfig.logLevel = "info";
        logConfig.logPath = defaultLogPath.string();
        logConfig.maxFileSize = 1024 * 1024;
        logConfig.maxFiles = 3;
        WARNLOG("获取日志配置失败: {}", e.what());
    }
    return logConfig;
}

void ConfigManager::reload() {
    loadConfig(configPath_);
    INFOLOG("配置文件已重新加载: {}", configPath_);
}