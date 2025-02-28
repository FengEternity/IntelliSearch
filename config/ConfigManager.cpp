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
                    
                    // 将配置文件中的相对路径转换为绝对路径
                    std::filesystem::path configDir = std::filesystem::path(configPath_).parent_path();
                    if (config_.contains("api_providers")) {
                        for (auto& [provider, config] : config_["api_providers"].items()) {
                            if (config.contains("prompts")) {
                                // 处理嵌套的prompts对象
                                if (config["prompts"].is_object()) {
                                    for (auto& [prompt_type, prompt_path] : config["prompts"].items()) {
                                        if (prompt_path.is_string() && !std::filesystem::path(prompt_path.get<std::string>()).is_absolute()) {
                                            config["prompts"][prompt_type] = (configDir / prompt_path.get<std::string>()).string();
                                        }
                                    }
                                }
                                // 处理单个prompts字符串
                                else if (config["prompts"].is_string()) {
                                    std::string promptsPath = config["prompts"].get<std::string>();
                                    if (!std::filesystem::path(promptsPath).is_absolute()) {
                                        config["prompts"] = (configDir / promptsPath).string();
                                    }
                                }
                            }
                        }
                    }
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

// 新增方法：获取特定提供商的特定类型的提示文件路径
std::string ConfigManager::getProviderPromptPath(const std::string& provider, const std::string& promptType) const {
    try {
        const auto& providerConfig = getApiProviderConfig(provider);
        if (providerConfig.contains("prompts")) {
            if (providerConfig["prompts"].is_object() && providerConfig["prompts"].contains(promptType)) {
                return providerConfig["prompts"][promptType].get<std::string>();
            }
            // 如果prompts是字符串，则直接返回
            if (providerConfig["prompts"].is_string()) {
                return providerConfig["prompts"].get<std::string>();
            }
        }
        throw std::runtime_error("Prompt configuration not found for provider '" + provider + "' and type '" + promptType + "'");
    } catch (const std::exception& e) {
        WARNLOG("获取API提供商 {} 的 {} 提示配置失败: {}", provider, promptType, e.what());
        throw;
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

nlohmann::json ConfigManager::getApiProviderConfig(const std::string& provider) const {
    try {
        if (config_.contains("api_providers") && config_["api_providers"].contains(provider)) {
            return config_["api_providers"][provider];
        }
        throw std::runtime_error("API provider '" + provider + "' not found in config.json");
    } catch (const std::exception& e) {
        WARNLOG("获取API提供商 {} 的配置失败: {}", provider, e.what());
        throw;
    }
    return nlohmann::json();
}

nlohmann::json ConfigManager::getAllApiProviders() const {
    try {
        if (config_.contains("api_providers")) {
            return config_["api_providers"];
        }
    } catch (const std::exception& e) {
        WARNLOG("获取所有API提供商配置失败: {}", e.what());
    }
    return nlohmann::json();
}

void ConfigManager::reload() {
    loadConfig(configPath_);
    INFOLOG("配置文件已重新加载: {}", configPath_);
}