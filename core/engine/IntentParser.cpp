#include "IntentParser.h"
#include "../../log/Logger.h"
#include "../api/AIServiceManager.h"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include "SearchEngine.h"

namespace IntelliSearch {

IntentParser::IntentParser() {
    INFOLOG("Starting to initialize IntentParser");
    // 获取APIServiceManager实例
    aiServiceManager = AIServiceManager::getInstance();
    if (!aiServiceManager) {
        CRITICALLOG("Failed to get AIServiceManager instance");
        throw std::runtime_error("Failed to initialize AIServiceManager");
    }
}

IntentParser::~IntentParser() = default;

/*
 * Summary: 从搜索栏获取用户输入并解析意图
 * @param userInput 用户输入
 * @return nlohmann::json 意图解析结果
 */
nlohmann::json IntentParser::parseSearchIntent(const std::string& userInput) {
    DEBUGLOG("Received Intent parser request: {}", userInput);
    
    // 直接使用API进行意图解析
    auto result = aiServiceManager->parseIntent(userInput);
    
    // 使用搜索引擎执行搜索
    // auto searchResults = SearchEngine::getInstance()->performSearch(result);

    // 将搜索结果添加到意图解析结果中
    // result["search_results"] = searchResults;
    
    return result;
}

/*
 * Summary: 调用博查API进行搜索
 * @param query 搜索查询字符串
 * @return nlohmann::json 搜索结果
 */
nlohmann::json IntentParser::bochaSearch(const std::string &query) {
    DEBUGLOG("Received search request: {}", query);

    auto searchResults = SearchEngine::getInstance()->performSearch(query);

    return searchResults;
}

} // namespace IntelliSearch