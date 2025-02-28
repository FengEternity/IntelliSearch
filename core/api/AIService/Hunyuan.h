//
// Created by montee on 25-2-24.
//

#ifndef HUNYUAN_H
#define HUNYUAN_H

#include "AIService.h"
#include "../../../log/Logger.h"

namespace IntelliSearch {
    class Hunyuan : public AIService{
        public:
            Hunyuan();
            ~Hunyuan();

            nlohmann::json parseIntent(const std::string& userInput) override;
            nlohmann::json searchParser(const std::string& userInput) override;
            std::string getServiceName() const override { return "Hunyuan"; }
            bool isAvailable() const override { return curl != nullptr && !apiKey.empty(); }
            int getPriority() const override { return 1; }

        protected:
            void handleError(const std::string& error) override { ERRORLOG("HunyuanAPIService error: {}", error); }
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

#endif //HUNYUAN_H
