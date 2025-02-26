#include "SearchEngine.h"
#include "../../log/Logger.h"
#include "../api/SearchService/Bocha.h"
#include "../api/SearchServiceManager.h"
#include "../api/AIServiceManager.h"

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
    aiServiceManager = AIServiceManager::getInstance();
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

        // 调用AI服务进行分析总结
        nlohmann::json analysis = analyzeSearchResults(response, intentResult);
        response["analysis"] = analysis;

        return response;
        
    } catch (const std::exception& e) {
        ERRORLOG("Search failed: {}", e.what());
        return nlohmann::json{{"error", e.what()}};
    }
}

nlohmann::json SearchEngine::analyzeSearchResults(const nlohmann::json& searchResults, const std::string& userQuery) {
    try {
        INFOLOG("Analyzing search results for query: {}", userQuery);

        // 构建提示信息
        std::string prompt = "请根据以下搜索结果，对用户查询进行分析和总结：\n\n";
        prompt += "用户查询：" + userQuery + "\n\n";
        prompt += "搜索结果：\n";

        // 添加网页结果到提示信息
        if (searchResults.contains("webPages") && !searchResults["webPages"].empty()) {
            for (const auto& page : searchResults["webPages"]) {
                prompt += "- 标题：" + page["title"].get<std::string>() + "\n";
                prompt += "  摘要：" + page["snippet"].get<std::string>() + "\n\n";
            }
        }

        // 获取AI服务并进行分析
        AIService* aiService = aiServiceManager->getPreferredService();
        if (!aiService) {
            throw std::runtime_error("No available AI service");
        }

        // 调用AI服务进行分析
        nlohmann::json analysis = aiService->parseIntent(prompt);
        
        return analysis;

    } catch (const std::exception& e) {
        ERRORLOG("Analysis failed: {}", e.what());
        return nlohmann::json{{"error", e.what()}};
    }
}

} // namespace IntelliSearch 