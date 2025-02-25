#include "SearchEngine.h"
#include "../../log/Logger.h"
#include "../api/SearchService/Bocha.h"

namespace IntelliSearch {

std::unique_ptr<SearchEngine> SearchEngine::instance = nullptr;

SearchEngine* SearchEngine::getInstance() {
    if (!instance) {
        instance = std::unique_ptr<SearchEngine>(new SearchEngine());
    }
    return instance.get();
}

SearchEngine::SearchEngine() {
    // 获取服务管理器实例
    searchServiceManager = SearchServiceManager::getInstance();
}

SearchEngine::~SearchEngine() = default;

nlohmann::json SearchEngine::performSearch(const std::string& intentResult) {
    try {
        INFOLOG("Performing search for intentResult: {}", intentResult);
        
        // 获取首选搜索服务
        // auto* searchService = serviceManager->getPreferredService();
        // if (!searchService) {
        //     throw std::runtime_error("No available search service found");
        // }
        
        // 使用选定的服务执行搜索
        return searchServiceManager->performSearch(intentResult);
        
    } catch (const std::exception& e) {
        ERRORLOG("Search failed: {}", e.what());
        return nlohmann::json{{"error", e.what()}};
    }
}

} // namespace IntelliSearch 