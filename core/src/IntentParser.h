#ifndef INTELLISEARCH_INTENTPARSER_H
#define INTELLISEARCH_INTENTPARSER_H

#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

namespace IntelliSearch {

class IntentParser {
public:
    IntentParser();
    ~IntentParser();

    // 从搜索栏获取用户输入并解析意图
    nlohmann::json parseSearchIntent(const std::string& userInput);

private:
    // 调用 Kimi API 进行意图解析
    nlohmann::json callKimiAPI(const std::string& query);
    
    // 处理 API 响应
    nlohmann::json processApiResponse(const std::string& response);

    // 处理API请求重试
    nlohmann::json retryApiCall(const std::string& query, int attempt = 0);

    // CURL 写回调函数
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);

    // 获取当前时间戳（毫秒）
    static int64_t getCurrentTimeMs();

    // CURL 句柄
    CURL* curl;
    
    // API 密钥
    std::string apiKey;

    // API请求计数器
    int requestCount;
    
    // 上次重置计数器的时间
    int64_t lastResetTime;
};

} // namespace IntelliSearch

#endif // INTELLISEARCH_INTENTPARSER_H