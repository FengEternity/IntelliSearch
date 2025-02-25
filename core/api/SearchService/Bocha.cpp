#include "Bocha.h"
#include "../../log/Logger.h"
#include "../../config/ConfigManager.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>

namespace IntelliSearch {

Bocha::Bocha() {
    // 从配置文件获取API密钥
    auto* config = ConfigManager::getInstance();
    apiKey = config->getApiProviderConfig("bocha")["api_key"].get<std::string>();
    baseUrl = config->getApiProviderConfig("bocha")["base_url"].get<std::string>();
    maxResults = config->getApiProviderConfig("bocha")["max_results"].get<int>();
    timeoutMs = config->getApiProviderConfig("bocha")["timeout_ms"].get<int>();
    
    if (apiKey.empty()) {
        WARNLOG("Bocha API key not found in configuration");
    }

    if (baseUrl.empty()) {
        WARNLOG("Bocha base URL not found in configuration");
    }

    if (maxResults <= 0) {
        WARNLOG("Bocha max results must be greater than 0");
    }

    if (timeoutMs <= 0) {
        WARNLOG("Bocha timeout must be greater than 0");
    }
}

Bocha::~Bocha() = default;

// 静态回调函数用于接收响应数据
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

nlohmann::json Bocha::performSearch(const std::string& intentResult) {
    try {
        // 从 intentResult 中正确提取 query 字段
        std::string query = intentResult;
        std::string freshness = "oneYear";  // 默认值

        // // 根据意图调整搜索参数
        // if (intentResult["intent"] == "time_sensitive_query") {
        //     freshness = "day";
        // }
        
        
        return search(query, freshness, false, 10); // 待优化，根据意图调整搜索参数
    } catch (const std::exception& e) {
        ERRORLOG("Search failed: {}", e.what());
        throw;
    }
}
        
nlohmann::json Bocha::search(const std::string& query,
                              const std::string& freshness,
                              bool summary,
                              int count) {
    
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

void Bocha::handleError(const std::string& error) {
    ERRORLOG("Bocha service error: {}", error);
}

bool Bocha::validateApiKey() const {
    if (apiKey.empty()) {
        WARNLOG("Bocha API key is empty");
        return false;
    }
    // TODO: 实现实际的API密钥验证逻辑
    return true;
}

} // namespace IntelliSearch