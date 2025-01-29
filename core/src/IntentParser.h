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

    // CURL 写回调函数
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);

    // CURL 句柄
    CURL* curl;
    
    // API 密钥
    std::string apiKey;
};

} // namespace IntelliSearch

#endif // INTELLISEARCH_INTENTPARSER_H