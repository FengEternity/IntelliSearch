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
        // 指定使用 intent_parser prompt
        return callAPI(userInput, "intent_parser");
    } catch (const std::exception& e) {
        handleError(e.what());
        throw;
    }
}

nlohmann::json Kimi::searchParser(const std::string& userInput) {
    // 验证API密钥
    if (!validateApiKey()) {
        handleError("Invalid API key");
        throw std::runtime_error("Invalid API key");
    }
    try {
        return callAPI(userInput, "search_parser");
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

nlohmann::json Kimi::callAPI(const std::string& query, const std::string& promptType) {
    return retryApiCall(query, promptType);
}

// 修改 executeApiCall 方法
nlohmann::json Kimi::executeApiCall(const std::string& query, const std::string& promptType) {
    try {
        requestCount++;
        const std::string apiUrl = baseUrl +  "/v1/chat/completions";
        std::string response;

        // 使用基类方法设置基本的CURL参数
        setupBasicCurlOptions(apiUrl, &response);

        // 使用基类方法设置请求头
        std::string authHeader = "Authorization: Bearer " + apiKey;
        struct curl_slist* headers = setupRequestHeaders("Content-Type: application/json", authHeader);

        // 构建请求体
        auto* config = ConfigManager::getInstance();
        nlohmann::json requestBody = buildRequestBody(query, promptType, config);

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
        return processApiResponse(response, promptType);  // 添加promptType参数
    } catch (const std::exception& e) {
        throw;
    }
}

/*
* Summary: 构建API请求体
* Parameters:
*   const std::string& query - 用户查询
*   const std::string& promptType - 提示类型
*   ConfigManager* config - 配置管理器实例
* Returns:
*   nlohmann::json - 构建好的请求体
*/
nlohmann::json Kimi::buildRequestBody(const std::string& query, const std::string& promptType, ConfigManager* config) {
    nlohmann::json requestBody = {
        {"model", model},
        {"messages", nlohmann::json::array()},
        {"temperature", 0.3},
        {"max_tokens", 4096},
        {"response_format", config->getApiProviderConfig("kimi")["response_format"]}
    };

    // 如果指定了 promptType，则加载对应的 prompt
    if (!promptType.empty()) {
        std::string promptsFilePath = config->getProviderPromptPath("kimi", promptType);
        DEBUGLOG("加载提示文件: {}, 提示类型: {}", promptsFilePath, promptType);
        auto promptsJson = loadPromptsFile(promptsFilePath);

        if(promptType == "intent_parser") {
            requestBody["messages"] = nlohmann::json::array({
                {{"role", "system"}, {"content", utf8_encode(promptsJson["system"].dump())}},
                {{"role", "user"}, {"content", "示例输入：" + promptsJson["examples"]["input"].get<std::string>() + 
                                              "\n示例输出：" + promptsJson["examples"]["output"].dump() + 
                                              "\n\n实际输入：" + utf8_encode(query)}}
            });
        } else if(promptType == "search_parser") {
            requestBody["messages"] = nlohmann::json::array({
                {{"role", "system"}, {"content", utf8_encode(promptsJson["system"].dump())}},
                {{"role", "user"}, {"content", "示例输入：" + promptsJson["examples"]["input"].dump() +
                                              "\n示例输出：" + promptsJson["examples"]["output"].dump() +
                                              "\n\n实际输入：" + utf8_encode(query)}}
            });
        }
    } else {
        // 普通聊天模式
        requestBody["messages"] = nlohmann::json::array({
            {{"role", "user"}, {"content", utf8_encode(query)}}
        });
    }
    
    return requestBody;
}

nlohmann::json Kimi::processApiResponse(const std::string& response) {
    // 调用双参数版本，默认使用空的promptType
    return processApiResponse(response, "");
}

nlohmann::json Kimi::processApiResponse(const std::string& response, const std::string& promptType) {
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
        
        // 根据不同的promptType处理返回结果
        if (promptType == "search_parser") {
            return processSearchParserResponse(content);
        } else {
            // 默认处理方式（适用于intent_parser）
            return nlohmann::json::parse(content);
        }
    } catch (const nlohmann::json::exception& e) {
        ERRORLOG("JSON parsing error: {}", e.what());
        throw std::runtime_error(std::string("JSON parsing error: ") + e.what());
    } catch (const std::exception& e) {
        ERRORLOG("Error processing API response: {}", e.what());
        throw;
    }
}

nlohmann::json Kimi::processSearchParserResponse(const std::string& content) {
    try {
        auto jsonContent = nlohmann::json::parse(content);
        
        // 验证搜索解析结果的必要字段
        if (!jsonContent.contains("relevance") || 
            !jsonContent.contains("credibility") || 
            !jsonContent.contains("completeness") || 
            !jsonContent.contains("parsed_data") ||
            !jsonContent.contains("result") ||
            !jsonContent.contains("summary")) {
            ERRORLOG("Invalid search parser response format");
            throw std::runtime_error("Invalid search parser response format");
        }
        
        return jsonContent;
    } catch (const nlohmann::json::exception& e) {
        ERRORLOG("Search parser response parsing error: {}", e.what());
        throw std::runtime_error(std::string("Search parser response parsing error: ") + e.what());
    }
}

} // namespace IntelliSearch