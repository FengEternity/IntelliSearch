#ifndef INTELLISEARCH_KIMIAPISERVICE_H
#define INTELLISEARCH_KIMIAPISERVICE_H

#include "BaseAPIService.h"

namespace IntelliSearch {

class KimiAPIService : public BaseAPIService {
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
    
    // 执行API调用
    nlohmann::json executeApiCall(const std::string& query) override;
    
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

#endif // INTELLISEARCH_KIMIAPISERVICE_H