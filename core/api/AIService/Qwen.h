#ifndef INTELLISEARCH_QWEN_H
#define INTELLISEARCH_QWEN_H

#include "AIService.h"

namespace IntelliSearch {

class Qwen : public AIService {
public:
    Qwen();
    ~Qwen();

    nlohmann::json parseIntent(const std::string& userInput) override;
    nlohmann::json searchParser(const std::string& userInput) override;
    std::string getServiceName() const override { return "Qwen"; }
    bool isAvailable() const override;
    int getPriority() const override { return 1; }

protected:
    void handleError(const std::string& error) override;
    bool validateApiKey() const override;

private:
    nlohmann::json callAPI(const std::string& query, const std::string& promptType = "") { return retryApiCall(query, promptType);};
    nlohmann::json executeApiCall(const std::string& query, const std::string& promptType) override;
    nlohmann::json processApiResponse(const std::string& response) override;

    std::string apiKey;
    std::string baseUrl;
    std::string model;
};

} // namespace IntelliSearch

#endif