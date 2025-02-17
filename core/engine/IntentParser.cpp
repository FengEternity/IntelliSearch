#include "IntentParser.h"
#include "../../log/Logger.h"
#include "../api/APIServiceManager.h"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include "SearchEngine.h"

namespace IntelliSearch {

IntentParser::IntentParser() {
    INFOLOG("Starting to initialize IntentParser");
    // 获取APIServiceManager实例
    apiServiceManager = APIServiceManager::getInstance();
    if (!apiServiceManager) {
        CRITICALLOG("Failed to get APIServiceManager instance");
        throw std::runtime_error("Failed to initialize APIServiceManager");
    }
}

IntentParser::~IntentParser() {
    // 析构函数不需要特殊清理
}

nlohmann::json IntentParser::parseSearchIntent(const std::string& userInput) {
    DEBUGLOG("Received search request: {}", userInput);
    
    // 直接使用API进行意图解析
    auto result = apiServiceManager->parseIntent(userInput);
    
    // 使用搜索引擎执行搜索
    auto searchResults = SearchEngine::getInstance()->performSearch(result);
    
    // 将搜索结果添加到意图解析结果中
    result["search_results"] = searchResults;
    
    return result;
}

} // namespace IntelliSearch