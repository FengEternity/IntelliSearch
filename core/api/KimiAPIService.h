#ifndef INTELLISEARCH_KIMIAPISERVICE_H
#define INTELLISEARCH_KIMIAPISERVICE_H

#include "APIService.h"
#include <curl/curl.h>

namespace IntelliSearch {

class KimiAPIService : public APIService {
public:
    KimiAPIService();
    ~KimiAPIService();

    // 实现 APIService 接口
    nlohmann::json parseIntent(const std::string& userInput) override;
    std::string getServiceName() const override { return "Kimi"; }
    bool isAvailable() const override;
    int getPriority() const override { return 1; }

protected:
    void handleError(const std::string& error) override;
    bool validateApiKey() const override;

private:
    // 调用 Kimi API
    nlohmann::json callAPI(const std::string& query);
    
    // 处理 API 响应
    nlohmann::json processApiResponse(const std::string& response);

    // 处理API请求重试
    nlohmann::json retryApiCall(const std::string& query, int attempt = 0);

    // CURL 写回调函数
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);

    // 获取当前时间戳（毫秒）
    static int64_t getCurrentTimeMs();

    // UTF-8编码转换函数
    std::string utf8_encode(const std::string& str);
    
    // 检查字符串是否为有效的UTF-8编码
    bool is_valid_utf8(const std::string& str);

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

#endif // INTELLISEARCH_KIMIAPISERVICE_H