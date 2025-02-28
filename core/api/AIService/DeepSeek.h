//
// Created by montee on 25-2-25.
//

#ifndef DEEPSEEK_H
#define DEEPSEEK_H

#include "AIService.h"
#include "../../../log/Logger.h"

namespace IntelliSearch {
    class DeepSeek : public AIService {
      public:
        DeepSeek();
        ~DeepSeek();

        nlohmann::json parseIntent(const std::string& userInput) override;
        nlohmann::json searchParser(const std::string& userInput) override;
        std::string getServiceName() const override { return "DeepSeek"; }
        bool isAvailable() const override { return curl != nullptr && !apiKey.empty(); }
        int getPriority() const override { return 1; }

      protected:
        void handleError(const std::string& error) override { ERRORLOG("DeepSeekAPIService error: {}", error); }
        bool validateApiKey() const override { return !apiKey.empty(); }

    private:
        nlohmann::json callAPI(const std::string& query, const std::string& promptType = "") { return retryApiCall(query, promptType);};
        nlohmann::json executeApiCall(const std::string& query, const std::string& promptType) override;


        std::string apiKey;
        std::string baseUrl;
        std::string model;
        std::string intentPrompts;
        std::string responseFormat;
    };
}

#endif //DEEPSEEK_H
