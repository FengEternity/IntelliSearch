#include "IntentParser.h"
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <nlohmann/json.hpp>

namespace IntelliSearch {

IntentParser::IntentParser() {
    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    
    // 尝试从配置文件读取 API 密钥
    try {
        std::ifstream configFile("config.json");
        if (configFile.is_open()) {
            nlohmann::json config = nlohmann::json::parse(configFile);
            if (config.contains("api_key")) {
                apiKey = config["api_key"];
            }
        }
    } catch (const std::exception& e) {
        // 如果配置文件读取失败，记录错误但继续尝试环境变量
    }
    
    // 如果配置文件中没有找到 API 密钥，则尝试从环境变量获取
    if (apiKey.empty()) {
        apiKey = std::getenv("KIMI_API_KEY") ? std::getenv("KIMI_API_KEY") : "";
    }
    
    if (apiKey.empty()) {
        throw std::runtime_error("KIMI API key not found in config.json or environment variable");
    }
}

IntentParser::~IntentParser() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

nlohmann::json IntentParser::parseSearchIntent(const std::string& userInput) {
    if (userInput.empty()) {
        throw std::invalid_argument("User input cannot be empty");
    }
    
    try {
        return callKimiAPI(userInput);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to parse search intent: ") + e.what());
    }
}

nlohmann::json IntentParser::callKimiAPI(const std::string& query) {
    std::string readBuffer;
    
    // 设置 CURL 选项
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.moonshot.cn/v1/chat/completions");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_ENCODING, "UTF-8");
    
    // 设置请求头
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // 构建请求体
    nlohmann::json requestBody = {
        {"model", "moonshot-v1-8k"},
        {"messages", {
            {{"role", "system"}, {"content", "你是一个搜索意图解析助手，需要分析用户的搜索查询并提取关键信息。"}},
            {{"role", "user"}, {"content", query}}
        }},
        {"temperature", 0.3}
    };
    
    std::string requestStr = requestBody.dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestStr.c_str());
    
    // 发送请求
    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    
    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("CURL request failed: ") + curl_easy_strerror(res));
    }
    
    return processApiResponse(readBuffer);
}

nlohmann::json IntentParser::processApiResponse(const std::string& response) {
    try {
        if (response.empty()) {
            throw std::runtime_error("Empty API response");
        }

        // 解析 JSON 响应
        nlohmann::json jsonResponse = nlohmann::json::parse(response);
        
        // 提取 API 响应中的意图信息
        if (jsonResponse.contains("choices") && !jsonResponse["choices"].empty() &&
            jsonResponse["choices"][0].contains("message") &&
            jsonResponse["choices"][0]["message"].contains("content")) {
            
            std::string content = jsonResponse["choices"][0]["message"]["content"];
            if (!content.empty()) {
                return nlohmann::json::parse(content);
            }
        }
        
        throw std::runtime_error("Invalid API response format");
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error(std::string("Failed to parse API response: ") + e.what());
    }
}

size_t IntentParser::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

} // namespace IntelliSearch