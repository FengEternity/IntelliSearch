#include "IntentParser.h"
#include "../../log/Logger.h"
#include "../../config/ConfigManager.h"
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <nlohmann/json.hpp>

namespace IntelliSearch {

IntentParser::IntentParser() {
    INFOLOG("Starting to initialize IntentParser");
    curl = curl_easy_init();
    if (!curl) {
        CRITICALLOG("CURL initialization failed");
        throw std::runtime_error("Failed to initialize CURL");
    }
    
    // 从 ConfigManager 获取 API 密钥
    apiKey = ConfigManager::getInstance()->getStringValue("api_key");
    
    if (apiKey.empty()) {
        // 如果配置文件中没有，尝试从环境变量获取
        apiKey = std::getenv("KIMI_API_KEY") ? std::getenv("KIMI_API_KEY") : "";
    }
    
    if (apiKey.empty()) {
        CRITICALLOG("KIMI API key not found, please check config file or environment variables");
        throw std::runtime_error("KIMI API key not found in config.json or environment variable");
    }

    // 初始化API请求计数器
    requestCount = 0;
    lastResetTime = getCurrentTimeMs();
}

IntentParser::~IntentParser() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

nlohmann::json IntentParser::parseSearchIntent(const std::string& userInput) {
    DEBUGLOG("Received search request: {}", userInput);
    return retryApiCall(userInput);
}

int64_t IntentParser::getCurrentTimeMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

nlohmann::json IntentParser::retryApiCall(const std::string& query, int attempt) {
    auto config = ConfigManager::getInstance();
    int maxAttempts = config->getIntValue("api/retry/max_attempts", 3);
    int initialDelay = config->getIntValue("api/retry/initial_delay_ms", 1000);
    int maxDelay = config->getIntValue("api/retry/max_delay_ms", 5000);
    int maxRequestsPerMinute = config->getIntValue("api/max_requests_per_minute", 3);

    // 检查并重置请求计数器
    int64_t currentTime = getCurrentTimeMs();
    if (currentTime - lastResetTime >= 60000) { // 1分钟
        requestCount = 0;
        lastResetTime = currentTime;
    }

    // 检查请求速率限制
    if (requestCount >= maxRequestsPerMinute) {
        int64_t waitTime = 60000 - (currentTime - lastResetTime);
        if (waitTime > 0) {
            WARNLOG("Rate limit reached, waiting for {} ms", waitTime);
            std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
            requestCount = 0;
            lastResetTime = getCurrentTimeMs();
        }
    }

    try {
        requestCount++;
        return callKimiAPI(query);
    } catch (const std::exception& e) {
        if (attempt < maxAttempts) {
            int delay = std::min(initialDelay * (1 << attempt), maxDelay);
            WARNLOG("API call failed, retrying in {} ms (attempt {}/{}): {}", delay, attempt + 1, maxAttempts, e.what());
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            return retryApiCall(query, attempt + 1);
        }
        throw;
    }
}

nlohmann::json IntentParser::callKimiAPI(const std::string& query) {
    std::string readBuffer;
    
    // Set CURL options
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.moonshot.cn/v1/chat/completions");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_ENCODING, "UTF-8");
    
    // Set request headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Build request body
    nlohmann::json requestBody = {
        {"model", "moonshot-v1-8k"},
        {"messages", {
            {{"role", "system"}, {"content", "You are a search intent parsing assistant, responsible for analyzing user search queries and extracting key information."}},
            {{"role", "user"}, {"content", query}}
        }},
        {"temperature", 0.3}
    };
    
    std::string requestStr = requestBody.dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestStr.c_str());
    
    // Send request
    CURLcode res = curl_easy_perform(curl);
    
    // Clean up request headers
    curl_slist_free_all(headers);
    
    if (res != CURLE_OK) {
        ERRORLOG("CURL request failed: {}", curl_easy_strerror(res));
        throw std::runtime_error(std::string("CURL request failed: ") + curl_easy_strerror(res));
    }
    
    DEBUGLOG("Raw API response data: {}", readBuffer);
    return processApiResponse(readBuffer);
}

nlohmann::json IntentParser::processApiResponse(const std::string& response) {
    if (response.empty()) {
        ERRORLOG("API response is empty");
        throw std::runtime_error("Empty API response");
    }
    
    DEBUGLOG("Starting to parse API response, response length: {}", response.length());
    try {
        nlohmann::json jsonResponse = nlohmann::json::parse(response);
        DEBUGLOG("API response JSON parsed successfully, starting to extract content");
        
        if (!jsonResponse.contains("choices") || jsonResponse["choices"].empty() ||
            !jsonResponse["choices"][0].contains("message") ||
            !jsonResponse["choices"][0]["message"].contains("content")) {
            ERRORLOG("Invalid API response format: {}", response);
            throw std::runtime_error("Invalid API response format");
        }
        
        std::string content = jsonResponse["choices"][0]["message"]["content"];
        INFOLOG("Successfully extracted API response content: {}", content);
        
        // 创建一个包含解析结果的 JSON 对象
        nlohmann::json result = {
            {"content", content},
            {"type", "text"}
        };
        
        return result;
    } catch (const nlohmann::json::parse_error& e) {
        ERRORLOG("Failed to parse API response JSON: {}", e.what());
        throw std::runtime_error(std::string("Failed to parse API response: ") + e.what());
    }
}

size_t IntentParser::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

} // namespace IntelliSearch