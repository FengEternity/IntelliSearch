//
// Created by montee on 25-2-25.
//

//
// Created by montee on 25-2-24.
//

#include "DeepSeek.h"
#include "../../../config/ConfigManager.h"

namespace IntelliSearch {
    DeepSeek::DeepSeek() {
        auto* config = ConfigManager::getInstance();
        apiKey = config->getApiProviderConfig("deepseek")["api_key"].get<std::string>();
        baseUrl = config->getApiProviderConfig("deepseek")["base_url"].get<std::string>();
        model = config->getApiProviderConfig("deepseek")["model"].get<std::string>();
        intentPrompts = config->getProviderPromptPath("deepseek", "intent_parser");

    }

    DeepSeek::~DeepSeek() = default;

    nlohmann::json DeepSeek::parseIntent(const std::string& userInput) {
        // 验证API密钥
        if (!validateApiKey()) {
            handleError("Invalid API key");
            throw std::runtime_error("Invalid API key");
        }

        try {
            return callAPI(userInput);
        } catch (const std::exception& e) {
            handleError(e.what());
            throw;
        }
    }

    nlohmann::json DeepSeek::executeApiCall(const std::string& query) {
        try {
            requestCount++;
            const std::string apiUrl = baseUrl + "/chat/completions";
            std::string response;

            setupBasicCurlOptions(apiUrl, &response);
            std::string authHeader = "Authorization: Bearer " + apiKey;
            struct curl_slist* headers = setupRequestHeaders("Content-Type: application/json", authHeader);

            auto promptsJson = loadPromptsFile(intentPrompts);

            nlohmann::json requestBody = {
                {"model", model},
                {"messages", nlohmann::json::array({
                    {{"role", "system"}, {"content", utf8_encode(promptsJson["system"].dump())}},
                    {{"role", "user"}, {"content", "示例输入：" + promptsJson["examples"]["input"].get<std::string>() +
                                                  "\n示例输出：" + promptsJson["examples"]["output"].dump() +
                                                  "\n\n实际输入：" + utf8_encode(query)}}
                })}
            };

            std::string requestBodyStr = requestBody.dump();
            INFOLOG("Sending API request with content: {}", requestBodyStr);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBodyStr.c_str());

            // 发送请求
            CURLcode res = curl_easy_perform(curl);
            curl_slist_free_all(headers);

            if (res != CURLE_OK) {
                ERRORLOG("CURL request failed: {}", curl_easy_strerror(res));
                throw std::runtime_error(std::string("CURL request failed: ") + curl_easy_strerror(res));
            }

            INFOLOG("Received API response: {}", response);
            return processApiResponse(response);
        } catch (const std::exception& e) {
            throw;
        }
    }

} // IntelliSearch

