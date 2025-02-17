#include "SearchEngine.h"
#include "../../log/Logger.h"
#include <curl/curl.h>
#include <sstream>

namespace IntelliSearch {

// 用于接收 CURL 响应数据的回调函数
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::unique_ptr<SearchEngine> SearchEngine::instance = nullptr;

SearchEngine* SearchEngine::getInstance() {
    if (!instance) {
        instance = std::unique_ptr<SearchEngine>(new SearchEngine());
    }
    return instance.get();
}

SearchEngine::SearchEngine() {
    loadConfig();
    // 初始化 CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

SearchEngine::~SearchEngine() {
    // 清理 CURL
    curl_global_cleanup();
}

void SearchEngine::loadConfig() {
    try {
        auto config = ConfigManager::getInstance()->getApiProviderConfig("bocha");
        apiKey = config["api_key"].get<std::string>();
        baseUrl = config["base_url"].get<std::string>();
        maxResults = config.value("max_results", 8);
        timeoutMs = config.value("timeout_ms", 5000);
    } catch (const std::exception& e) {
        ERRORLOG("Failed to load SearchEngine config: {}", e.what());
        throw;
    }
}

nlohmann::json SearchEngine::performSearch(const nlohmann::json& intentResult) {
    try {
        INFOLOG("Performing Bocha search for intentResult: {}", intentResult.dump());
        
        // 从 intentResult 中正确提取 query 字段
        if (!intentResult.contains("query")) {
            throw std::runtime_error("Missing 'query' field in intentResult");
        }
        std::string query = intentResult["query"].get<std::string>();
        std::string freshness = "oneYear";  // 默认值
        
        // 根据意图调整搜索参数
        if (intentResult["intent"] == "time_sensitive_query") {
            freshness = "day";
        }
        
        return searchWithBocha(query, freshness);
    } catch (const std::exception& e) {
        ERRORLOG("Search failed: {}", e.what());
        return nlohmann::json{{"error", e.what()}};
    }
}

nlohmann::json SearchEngine::searchWithBocha(const std::string& query,
                                           const std::string& freshness,
                                           bool summary,
                                           int count) {
    INFOLOG("Performing Bocha search for query: {}", query);
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    
    std::string readBuffer;
    struct curl_slist* headers = NULL;
    
    try {
        // 准备请求体
        INFOLOG("Performing Bocha search for query: {}", query);
        nlohmann::json requestBody = {
            {"query", query},
            {"freshness", freshness},
            {"summary", summary},
            {"count", count}
        };
        std::string jsonBody = requestBody.dump();
        
        // 设置请求头
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string authHeader = "Authorization: Bearer " + apiKey;
        headers = curl_slist_append(headers, authHeader.c_str());
        
        // 配置 CURL
        std::string url = baseUrl + "/web-search";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeoutMs);
        
        // 执行请求
        CURLcode res = curl_easy_perform(curl);
        
        // 检查响应状态
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        
        // 清理
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) {
            ERRORLOG("Curl request failed: {}", curl_easy_strerror(res));
            throw std::runtime_error("Search API request failed");
        }
        
        if (httpCode != 200) {
            ERRORLOG("Bocha API request failed with status code: {} - {}", httpCode, readBuffer);
            throw std::runtime_error("Search API request failed");
        }
        
        // 添加API返回结果的日志
        INFOLOG("Bocha API response: {}", readBuffer);
        
        return nlohmann::json::parse(readBuffer);
        
    } catch (const std::exception& e) {
        if (headers) curl_slist_free_all(headers);
        if (curl) curl_easy_cleanup(curl);
        ERRORLOG("Search request failed: {}", e.what());
        throw;
    }
}

} // namespace IntelliSearch 