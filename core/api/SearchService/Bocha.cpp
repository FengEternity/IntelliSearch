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
        bool summary = false;
        int count = 10;


        // // 根据意图调整搜索参数
        // if (intentResult["intent"] == "time_sensitive_query") {
        //     freshness = "day";
        // }

        nlohmann::json response = search(query, freshness, summary, count);

        return response; // 待优化，根据意图调整搜索参数
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
    return true;
}

SearchResults Bocha::processSearchResults(const nlohmann::json& response) {
    SearchResults results;

    try {
        // 检查响应结构有效性
        if (!response.contains("data") || !response["data"].is_object()) {
            throw std::runtime_error("Invalid API response structure");
        }

        const auto& data = response["data"];

        // 处理网页结果
        if (data.contains("webPages") && data["webPages"].is_object()) {
            const auto& webPages = data["webPages"];

            if (webPages.contains("value") && webPages["value"].is_array()) {
                for (const auto& item : webPages["value"]) {
                    WebPageResult page;
                    if (item.contains("name")) page.title = item["name"].get<std::string>();
                    if (item.contains("url")) page.url = item["url"].get<std::string>();
                    if (item.contains("snippet")) page.snippet = item["snippet"].get<std::string>();
                    if (item.contains("siteName")) page.siteName = item["siteName"].get<std::string>();
                    if (item.contains("dateLastCrawled")) page.date = item["dateLastCrawled"].get<std::string>();

                    results.webPages.push_back(page);
                }
            }

            if (webPages.contains("someResultsRemoved")) {
                results.hasFilteredResults = webPages["someResultsRemoved"].get<bool>();
            }
        }

        // 处理图片结果
        if (data.contains("images") && data["images"].is_object()) {
            const auto& images = data["images"];

            if (images.contains("value") && images["value"].is_array()) {
                for (const auto& item : images["value"]) {
                    ImageResult img;
                    if (item.contains("thumbnailUrl")) img.thumbnailUrl = item["thumbnailUrl"].get<std::string>();
                    if (item.contains("contentUrl")) img.contentUrl = item["contentUrl"].get<std::string>();

                    results.images.push_back(img);
                }
            }
        }

        // 记录处理结果摘要
        INFOLOG("Processed {} web results and {} image results",
                results.webPages.size(),
                results.images.size());

        // 详细日志调试模式
        DEBUGLOG("Search Results Details:\n{}\n{}",
                [&](){
                    std::stringstream ss;
                    ss << "\n=== Web Results ===";
                    for (size_t i = 0; i < results.webPages.size(); ++i) {
                        const auto& page = results.webPages[i];
                        ss << "\n[" << i+1 << "]"
                           << "\n    Title: " << (page.title.empty() ? "[No Title]" : page.title)
                           << "\n    URL: " << (page.url.empty() ? "[No URL]" : page.url)
                           << "\n    Snippet: " << (page.snippet.empty() ? "[No Snippet]" : page.snippet)
                           << "\n    Site Name: " << (page.siteName.empty() ? "[No Site Name]" : page.siteName)
                           << "\n    Date: " << (page.date.empty() ? "[No Date]" : page.date)
                           << "\n";
                    }
                    return ss.str();
                }(),
                [&](){
                    std::stringstream ss;
                    ss << "\n=== Image Results ===";
                    for (size_t i = 0; i < results.images.size(); ++i) {
                        const auto& img = results.images[i];
                        ss << "\n[" << i+1 << "]"
                           << "\n    Thumbnail URL: " << (img.thumbnailUrl.empty() ? "[No Thumbnail]" : img.thumbnailUrl)
                           << "\n    Content URL: " << (img.contentUrl.empty() ? "[No Content URL]" : img.contentUrl)
                           << "\n";
                    }
                    return ss.str();
                }());

        // 记录过滤状态
        if (results.hasFilteredResults) {
            WARNLOG("Partial results filtered by search provider");
        }


        return results;

    } catch (const std::exception& e) {
        ERRORLOG("Result processing failed: {}", e.what());
        throw;
    }
}

} // namespace IntelliSearch