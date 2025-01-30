#include "IntentParser.h"
#include "../../log/Logger.h"
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
    
    // Try to read API key from config file
    try {
        std::ifstream configFile("config.json");
        if (configFile.is_open()) {
            nlohmann::json config = nlohmann::json::parse(configFile);
            if (config.contains("api_key")) {
                apiKey = config["api_key"];
            }
        }
    } catch (const std::exception& e) {
        WARNLOG("Failed to read config file: {}", e.what());
        // If config file reading fails, log error and try environment variable
    }
    
    // If API key not found in config file, try to get from environment variable
    if (apiKey.empty()) {
        apiKey = std::getenv("KIMI_API_KEY") ? std::getenv("KIMI_API_KEY") : "";
    }
    
    if (apiKey.empty()) {
        CRITICALLOG("KIMI API key not found, please check config file or environment variables");
        throw std::runtime_error("KIMI API key not found in config.json or environment variable");
    }
}

IntentParser::~IntentParser() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

nlohmann::json IntentParser::parseSearchIntent(const std::string& userInput) {
    DEBUGLOG("Received search request: {}", userInput);
    if (userInput.empty()) {
        ERRORLOG("Search input is empty");
        throw std::invalid_argument("User input cannot be empty");
    }
    
    try {
        INFOLOG("Starting to call Kimi API to parse search intent");
        auto result = callKimiAPI(userInput);
        INFOLOG("Successfully parsed search intent");
        return result;
    } catch (const std::exception& e) {
        ERRORLOG("Failed to parse search intent: {}", e.what());
        throw std::runtime_error(std::string("Failed to parse search intent: ") + e.what());
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
        DEBUGLOG("Successfully extracted API response content: {}", content);
        
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