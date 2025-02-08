#include "IntentParser.h"
#include "../../log/Logger.h"
#include "../api/APIServiceManager.h"
#include <stdexcept>
#include <fstream>
#include <sstream>

namespace IntelliSearch {

IntentParser::IntentParser() {
    INFOLOG("Starting to initialize IntentParser");
    // 获取APIServiceManager实例
    apiServiceManager = APIServiceManager::getInstance();
    if (!apiServiceManager) {
        CRITICALLOG("Failed to get APIServiceManager instance");
        throw std::runtime_error("Failed to initialize APIServiceManager");
    }
    // 加载意图解析配置
    loadIntentConfig();
}

IntentParser::~IntentParser() {
    // 析构函数不需要特殊清理
}

nlohmann::json IntentParser::parseSearchIntent(const std::string& userInput) {
    DEBUGLOG("Received search request: {}", userInput);
    
    // 执行本地意图解析
    auto localResult = localIntentParsing(userInput);
    
    // 获取API意图解析结果
    auto apiResult = apiServiceManager->parseIntent(userInput);
    
    // 合并本地和API的解析结果
    return mergeIntentResults(localResult, apiResult);
}

nlohmann::json IntentParser::localIntentParsing(const std::string& input) {
    DEBUGLOG("Performing local intent parsing for: {}", input);
    nlohmann::json result;
    
    try {
        result["source"] = "local";
        result["confidence"] = 0.0f;
        result["entities"] = nlohmann::json::array();
        
        // 使用配置中的规则进行意图识别
        for (const auto& rule : intentConfig["rules"]) {
            std::string pattern = rule["pattern"].get<std::string>();
            size_t pos = 0;
            bool matched = false;
            
            // 检查每个关键词
            std::string currentPattern;
            std::istringstream patternStream(pattern);
            while (std::getline(patternStream, currentPattern, '|')) {
                if (input.find(currentPattern) != std::string::npos) {
                    matched = true;
                    // 将匹配到的关键词添加到实体中
                    result["entities"].push_back({
                        {"type", "keyword"},
                        {"value", currentPattern},
                        {"position", input.find(currentPattern)}
                    });
                    break;
                }
            }
            
            if (matched) {
                result["intent"] = rule["intent"].get<std::string>();
                result["confidence"] = rule["confidence"].get<float>();
                break;
            }
        }
        
        // 如果没有匹配到任何规则，设置默认意图
        if (result["confidence"].get<float>() == 0.0f) {
            result["intent"] = "knowledge_query";
            result["confidence"] = 0.5f;
        }
        
    } catch (const std::exception& e) {
        ERRORLOG("Local intent parsing failed: {}", e.what());
        result["error"] = {{
            "code", "E001"},
            {"detail", "本地意图解析失败"},
            {"suggestion", "请尝试重新输入或使用更简单的查询语句"}
        };
    }
    
    return result;
}

nlohmann::json IntentParser::mergeIntentResults(const nlohmann::json& localResult, const nlohmann::json& apiResult) {
    DEBUGLOG("Merging intent results");
    nlohmann::json mergedResult;
    
    try {
        // 根据置信度选择结果
        float localConfidence = localResult.value("confidence", 0.0f);
        float apiConfidence = apiResult.value("confidence", 0.0f);
        
        if (localConfidence > apiConfidence) {
            mergedResult = localResult;
            mergedResult["source"] = "local";
        } else {
            mergedResult = apiResult;
            mergedResult["source"] = "api";
        }
        
        // 合并实体识别结果
        if (localResult.contains("entities") && apiResult.contains("entities")) {
            mergedResult["entities"] = localResult["entities"];
            for (const auto& entity : apiResult["entities"]) {
                if (std::find(mergedResult["entities"].begin(), 
                             mergedResult["entities"].end(), 
                             entity) == mergedResult["entities"].end()) {
                    mergedResult["entities"].push_back(entity);
                }
            }
        }
        
    } catch (const std::exception& e) {
        ERRORLOG("Failed to merge intent results: {}", e.what());
        mergedResult = apiResult; // 失败时使用API结果作为后备
    }
    
    INFOLOG("Intent parsing completed, result: {}", mergedResult.dump());
    return mergedResult;
}

void IntentParser::loadIntentConfig() {
    DEBUGLOG("Loading intent parser configuration");
    try {
        // 从配置文件加载意图解析规则
        std::ifstream configFile("config/intent_rules.json");
        if (!configFile.is_open()) {
            throw std::runtime_error("Failed to open intent_rules.json");
        }
        intentConfig = nlohmann::json::parse(configFile);
        INFOLOG("Successfully loaded intent rules from config file");
        
    } catch (const std::exception& e) {
        ERRORLOG("Failed to load intent config: {}", e.what());
        throw std::runtime_error("Failed to load intent configuration");
    }
}

} // namespace IntelliSearch