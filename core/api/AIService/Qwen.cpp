#include "Qwen.h"
#include "../../../config/ConfigManager.h"
#include "../../../log/Logger.h"
#include <fstream>
#include <sstream>

namespace IntelliSearch {

Qwen::Qwen() {
    auto* config = ConfigManager::getInstance();
    apiKey = config->getApiProviderConfig("qwen")["api_key"].get<std::string>();
    baseUrl = config->getApiProviderConfig("qwen")["base_url"].get<std::string>();
    model = config->getApiProviderConfig("qwen")["model"].get<std::string>();
    
    if (apiKey.empty()) {
        WARNLOG("Qwen API key not found in configuration");
    }

    if (baseUrl.empty()) {
        WARNLOG("Qwen base URL not found in configuration");
    }

    if (model.empty()) {
        WARNLOG("Qwen model not found in configuration");
    }
}

Qwen::~Qwen() = default;

nlohmann::json Qwen::parseIntent(const std::string& userInput) {
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

void Qwen::handleError(const std::string& error) {
    ERRORLOG("Qwen API error: " + error);
}

bool Qwen::isAvailable() const {
    return !apiKey.empty();
}

bool Qwen::validateApiKey() const {
    return !apiKey.empty();
}

nlohmann::json Qwen::callAPI(const std::string& query) {
    return retryApiCall(query);
}

nlohmann::json Qwen::executeApiCall(const std::string& query) {
    try {
        requestCount++;
        const std::string apiUrl = baseUrl + "/api/v1/services/aigc/text-generation/generation";
        std::string response;

        // 使用基类方法设置基本的CURL参数
        setupBasicCurlOptions(apiUrl, &response);

        // 使用基类方法设置请求头
        std::string authHeader = "Authorization: Bearer " + apiKey;
        struct curl_slist* headers = setupRequestHeaders("Content-Type: application/json", authHeader);
        
        // 获取配置管理器实例并构建请求体
        auto* config = ConfigManager::getInstance();
        std::string promptsFilePath = config->getProviderPromptPath("qwen", "intent_parser");

        // 使用基类方法加载提示文件
        auto promptsJson = loadPromptsFile(promptsFilePath);

        // 构建请求体
        nlohmann::json requestBody = {
            {"model", model},
            {"input", {
                {"messages", nlohmann::json::array({
                    {{"role", "system"}, {"content", utf8_encode(promptsJson["system"].dump())}},
                    {{"role", "user"}, {"content", "示例输入：" + promptsJson["examples"]["input"].get<std::string>() + 
                                                  "\n示例输出：" + promptsJson["examples"]["output"].dump() + 
                                                  "\n\n实际输入：" + utf8_encode(query)}}
                })}
            }},
            {"parameters", {
                {"result_format", "message"}
            }}
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

nlohmann::json Qwen::processApiResponse(const std::string& response) {
    try {
        nlohmann::json responseJson = nlohmann::json::parse(response);
        
        // 检查必要的字段是否存在
        if (!responseJson.contains("output") || 
            !responseJson["output"].contains("choices") ||
            responseJson["output"]["choices"].empty() ||
            !responseJson["output"]["choices"][0].contains("message") ||
            !responseJson["output"]["choices"][0]["message"].contains("content")) {
            ERRORLOG("Missing required fields in API response");
            throw std::runtime_error("Invalid API response format");
        }

        // 获取content字段的内容
        std::string content = responseJson["output"]["choices"][0]["message"]["content"];
        
        // 提取JSON内容 (去除 ```json 和 ``` 标记)
        size_t jsonStart = content.find("{");
        size_t jsonEnd = content.rfind("}");
        
        if (jsonStart == std::string::npos || jsonEnd == std::string::npos) {
            ERRORLOG("Invalid JSON content format in response");
            throw std::runtime_error("Invalid API response format");
        }

        // 提取JSON部分
        std::string jsonStr = content.substr(jsonStart, jsonEnd - jsonStart + 1);
        
        // 验证提取的JSON是否有效并返回
        return nlohmann::json::parse(jsonStr);
        
    } catch (const nlohmann::json::exception& e) {
        ERRORLOG("Error processing API response: {}", e.what());
        throw std::runtime_error("Invalid API response format");
    }
}

} // namespace IntelliSearch