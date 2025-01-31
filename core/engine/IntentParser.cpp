#include "IntentParser.h"
#include "../../log/Logger.h"
#include "../api/APIServiceManager.h"
#include <stdexcept>

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
    return apiServiceManager->parseIntent(userInput);
}

} // namespace IntelliSearch