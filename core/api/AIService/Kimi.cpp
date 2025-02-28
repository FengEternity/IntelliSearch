#include "Kimi.h"
#include "../../../config/ConfigManager.h"
#include "../../../log/Logger.h"
#include <fstream>
#include <sstream>

namespace IntelliSearch {

Kimi::Kimi() {
    // 从配置文件获取API密钥
    auto* config = ConfigManager::getInstance();
    apiKey = config->getApiProviderConfig("kimi")["api_key"].get<std::string>();
    baseUrl = config->getApiProviderConfig("kimi")["base_url"].get<std::string>();
    model = config->getApiProviderConfig("kimi")["model"].get<std::string>();
    
    if (apiKey.empty()) {
        WARNLOG("Kimi API key not found in configuration");
    }

    if (baseUrl.empty()) {
        WARNLOG("Kimi base URL not found in configuration");
    }

    if (model.empty()) {
        WARNLOG("Kimi model not found in configuration");
    }
}

Kimi::~Kimi() = default;

nlohmann::json Kimi::parseIntent(const std::string& userInput) {
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

void Kimi::handleError(const std::string& error) {
    ERRORLOG("KimiAPIService error: {}", error);
}

bool Kimi::isAvailable() const {
    return curl != nullptr && !apiKey.empty();
}

bool Kimi::validateApiKey() const {
    return !apiKey.empty();
}

nlohmann::json Kimi::callAPI(const std::string& query) {
    return retryApiCall(query);
}

nlohmann::json Kimi::executeApiCall(const std::string& query) {
    try {
        requestCount++;
        const std::string apiUrl = baseUrl +  "/v1/chat/completions";
        std::string response;

        // 使用基类方法设置基本的CURL参数
        setupBasicCurlOptions(apiUrl, &response);

        // 使用基类方法设置请求头
        std::string authHeader = "Authorization: Bearer " + apiKey;
        struct curl_slist* headers = setupRequestHeaders("Content-Type: application/json", authHeader);

        // 获取配置管理器实例并构建请求体
        auto* config = ConfigManager::getInstance();
        std::string promptsFilePath = config->getProviderPromptPath("kimi", "intent_parser");
        
        // 使用基类方法加载提示文件
        auto promptsJson = loadPromptsFile(promptsFilePath);

        // 构建请求体
        nlohmann::json requestBody = {
            {"model", model},
            {"messages", nlohmann::json::array({
                {{"role", "system"}, {"content", utf8_encode(promptsJson["system"].dump())}},
                {{"role", "user"}, {"content", "示例输入：" + promptsJson["examples"]["input"].get<std::string>() + 
                                              "\n示例输出：" + promptsJson["examples"]["output"].dump() + 
                                              "\n\n实际输入：" + utf8_encode(query)}}
            })},
            {"temperature", 0.3},
            {"max_tokens", 800},
            {"response_format", config->getApiProviderConfig("kimi")["response_format"]}
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

nlohmann::json Kimi::processApiResponse(const std::string& response) {
    try {
        DEBUGLOG("Processing API response: {}", response);
        
        if (response.empty()) {
            ERRORLOG("Empty API response received");
            throw std::runtime_error("Empty API response");
        }

        auto jsonResponse = nlohmann::json::parse(response);
        
        // 检查API响应中是否包含错误信息
        if (jsonResponse.contains("error")) {
            std::string errorMessage = jsonResponse["error"]["message"].get<std::string>();
            ERRORLOG("API returned error: {}", errorMessage);
            throw std::runtime_error("API error: " + errorMessage);
        }
        
        // 提取并解析助手的回复
        if (!jsonResponse.contains("choices") || jsonResponse["choices"].empty() ||
            !jsonResponse["choices"][0].contains("message") ||
            !jsonResponse["choices"][0]["message"].contains("content")) {
            ERRORLOG("Invalid API response format");
            throw std::runtime_error("Invalid API response format");
        }
        
        std::string content = jsonResponse["choices"][0]["message"]["content"].get<std::string>();
        return nlohmann::json::parse(content);
        
    } catch (const nlohmann::json::exception& e) {
        ERRORLOG("JSON parsing error: {}", e.what());
        throw std::runtime_error(std::string("JSON parsing error: ") + e.what());
    } catch (const std::exception& e) {
        ERRORLOG("Error processing API response: {}", e.what());
        throw;
    }
}

} // namespace IntelliSearch