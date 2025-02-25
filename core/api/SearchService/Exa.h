//
// Created by montee on 25-2-25.
//

#ifndef EXA_H
#define EXA_H

#include "SearchService.h"
#include "../../log/Logger.h"

namespace IntelliSearch
{
    class Exa : public SearchService
    {
        public:
            Exa();
            ~Exa() override;

            // 实现 APIService 的纯虚函数
            std::string getServiceName() const override { return "Bocha"; }
            bool isAvailable() const override { return !apiKey.empty() && validateApiKey(); }
            int getPriority() const override { return 1; }
            void handleError(const std::string& error) override { ERRORLOG("Bocha service error: {}", error); };
            bool validateApiKey() const override;

            // 实现 SearchService 的纯虚函数
            nlohmann::json performSearch(const std::string& intentResult) override;
            nlohmann::json search(const std::string& query,
                const std::string& type,
                std::string category,
                bool text,
                int count);

            SearchResults processSearchResults(const nlohmann::json& response) override;

        private:
            std::string apiKey; // API 密钥
            std::string baseUrl; // 基础 URL
            int maxResults; // 最大结果数
            int timeoutMs; // 超时时间
    };
}

#endif //EXA_H
