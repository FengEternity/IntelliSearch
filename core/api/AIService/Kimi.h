#ifndef INTELLISEARCH_KIMI_H
#define INTELLISEARCH_KIMI_H

#include "AIService.h"

namespace IntelliSearch {

class Kimi : public AIService {
public:
    Kimi();
    ~Kimi();

    // 实现 APIService 接口
    nlohmann::json parseIntent(const std::string& userInput) override;
    virtual nlohmann::json searchParser(const std::string& userInput) override;
    std::string getServiceName() const override { return "Kimi"; }
    bool isAvailable() const override;
    int getPriority() const override { return 1; }

protected:
    nlohmann::json buildRequestBody(const std::string& query, const std::string& promptType, ConfigManager* config);
    void handleError(const std::string& error) override;
    bool validateApiKey() const override;

private:
    // 调用 Kimi API
    nlohmann::json callAPI(const std::string& query, const std::string& promptType = "");
    
    // 执行API调用
    nlohmann::json executeApiCall(const std::string& query, const std::string& promptType) override;
    
    // 处理 API 响应
    nlohmann::json processApiResponse(const std::string& response) override;

    // API 密钥
    std::string apiKey;
    // API基础URL
    std::string baseUrl;
    // API模型
    std::string model;
};

} // namespace IntelliSearch

#endif // INTELLISEARCH_KIMI_H