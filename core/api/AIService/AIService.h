#ifndef INTELLISEARCH_AISERVICE_H
#define INTELLISEARCH_AISERVICE_H

#include "../APIService.h"
#include "../../../log/Logger.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>

namespace IntelliSearch {

class AIService : public APIService {
public:
    // 构造函数
    AIService();

    // 析构函数
    virtual ~AIService();

    // 解析用户输入的意图
    virtual nlohmann::json parseIntent(const std::string& userInput) = 0;
    virtual nlohmann::json searchParser(const std::string& userInput) = 0;

    // 获取服务名称
    virtual std::string getServiceName() const = 0;

    // 检查服务是否可用
    virtual bool isAvailable() const = 0;

    // 获取服务优先级（用于负载均衡和故障转移）
    virtual int getPriority() const = 0;

protected:
    // API调用的通用错误处理
    virtual void handleError(const std::string& error) {
        ERRORLOG("AIService error: {}", error);
    }

    // 验证API密钥
    virtual bool validateApiKey() const = 0;

    // 通用的API调用重试逻辑
    nlohmann::json retryApiCall(const std::string& query, 
                               const std::string& promptType = "", 
                               int attempt = 0);
    
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

    // 执行实际的API调用, 子类必须实现
    virtual nlohmann::json executeApiCall(const std::string& query, const std::string& promptType) = 0;

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

#endif // INTELLISEARCH_AISERVICE_H
