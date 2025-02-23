#ifndef INTELLISEARCH_SEARCHSERVICE_H
#define INTELLISEARCH_SEARCHSERVICE_H

#include "../APIService.h"
#include <curl/curl.h>
#include <string>
#include <chrono>

namespace IntelliSearch {

class SearchService : public APIService {
public:
    // 构造函数
    SearchService();

    // 析构函数
    virtual ~SearchService();
    
    // 发送搜索请求
    virtual nlohmann::json performSearch(const std::string& intentResult) = 0;
    virtual nlohmann::json search(const std::string& query,
                                          const std::string& freshness,
                                          bool summary,
                                          int count) = 0;

protected:
    CURL* curl;  // CURL句柄
    int requestCount;  // 请求计数
    int64_t lastResetTime;  // 上次重置时间

    // 工具函数
    int64_t getCurrentTimeMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    // CURL相关的辅助函数
    void setupBasicCurlOptions(const std::string& url, std::string* response);
    struct curl_slist* setupRequestHeaders(const std::string& contentType, const std::string& authHeader);
};

} // namespace IntelliSearch

#endif // INTELLISEARCH_SEARCHSERVICE_H