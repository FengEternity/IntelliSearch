#include "SearchEngine.h"
#include "../../log/Logger.h"
#include "../api/SearchService/Bocha.h"
#include "../api/SearchServiceManager.h"

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
        // 使用选定的服务执行搜索
        nlohmann::json searchResults = searchServiceManager->performSearch(intentResult);
        
        // 创建 Bocha 实例并处理搜索结果
        Bocha bochaService;
        SearchResults processedResults = bochaService.processSearchResults(searchResults);
        
        // 将处理后的结果转换为 JSON 格式
        nlohmann::json response = {
            {"webPages", nlohmann::json::array()},
            {"images", nlohmann::json::array()},
            {"hasFilteredResults", processedResults.hasFilteredResults}
        };

        // 添加网页结果
        for (const auto& page : processedResults.webPages) {
            response["webPages"].push_back({
                {"title", page.title},
                {"url", page.url},
                {"snippet", page.snippet},
                {"siteName", page.siteName},
                {"date", page.date}
            });
        }

        // 添加图片结果
        for (const auto& img : processedResults.images) {
            response["images"].push_back({
                {"thumbnailUrl", img.thumbnailUrl},
                {"contentUrl", img.contentUrl}
            });
        }

        return response;
        
    } catch (const std::exception& e) {
        ERRORLOG("Search failed: {}", e.what());
        return nlohmann::json{{"error", e.what()}};
    }
}


} // namespace IntelliSearch 