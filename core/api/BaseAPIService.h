#ifndef INTELLISEARCH_BASEAPISERVICE_H
#define INTELLISEARCH_BASEAPISERVICE_H

#include "APIService.h"
#include <curl/curl.h>
#include <chrono>
#include <string>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace IntelliSearch {

class BaseAPIService : public APIService {
protected:
    BaseAPIService();
    virtual ~BaseAPIService();

    // 通用的API调用重试逻辑
    nlohmann::json retryApiCall(const std::string& query, int attempt = 0);

    // CURL写回调函数
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);

    // 获取当前时间戳（毫秒）
    static int64_t getCurrentTimeMs();

    // UTF-8编码转换函数
    std::string utf8_encode(const std::string& str);
    
    // 检查字符串是否为有效的UTF-8编码
    bool is_valid_utf8(const std::string& str);

    // 处理API响应
    virtual nlohmann::json processApiResponse(const std::string& response);

    // 执行实际的API调用
    virtual nlohmann::json executeApiCall(const std::string& query) = 0;

    // 设置基本的CURL参数
    void setupBasicCurlOptions(const std::string& url, std::string* response);

    // 设置请求头
    curl_slist* setupRequestHeaders(const std::string& contentType, const std::string& authToken);

    // 查找并读取提示文件
    nlohmann::json loadPromptsFile(const std::string& promptsFilePath);

    // 构建搜索路径列表
    std::vector<std::filesystem::path> buildSearchPaths(const std::string& promptsFilePath);

    // CURL句柄
    CURL* curl;
    
    // API请求计数器
    int requestCount;
    
    // 上次重置计数器的时间
    int64_t lastResetTime;
};

} // namespace IntelliSearch

#endif // INTELLISEARCH_BASEAPISERVICE_H