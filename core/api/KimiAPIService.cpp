#include "KimiAPIService.h"
#include "../../config/ConfigManager.h"
#include "../../log/Logger.h"
#include <chrono>
#include <codecvt>
#include <locale>
#include <fstream>
#include <sstream>

namespace IntelliSearch {

KimiAPIService::KimiAPIService() : curl(nullptr), requestCount(0), lastResetTime(getCurrentTimeMs()) {
    curl = curl_easy_init();
    if (!curl) {
        ERRORLOG("Failed to initialize CURL in KimiAPIService");
        throw std::runtime_error("CURL initialization failed");
    }

    // 从配置文件获取API密钥
    auto* config = ConfigManager::getInstance();
    apiKey = config->getApiProviderConfig("kimi")["api_key"].get<std::string>();
    
    if (apiKey.empty()) {
        WARNLOG("Kimi API key not found in configuration");
    }
}

KimiAPIService::~KimiAPIService() {
    if (curl) {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
}

nlohmann::json KimiAPIService::parseIntent(const std::string& userInput) {
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

void KimiAPIService::handleError(const std::string& error) {
    ERRORLOG("KimiAPIService error: {}", error);
}

bool KimiAPIService::isAvailable() const {
    return curl != nullptr && !apiKey.empty();
}

bool KimiAPIService::validateApiKey() const {
    return !apiKey.empty();
}

int64_t KimiAPIService::getCurrentTimeMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

size_t KimiAPIService::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

nlohmann::json KimiAPIService::callAPI(const std::string& query) {
    return retryApiCall(query);
}

nlohmann::json KimiAPIService::retryApiCall(const std::string& query, int attempt) {
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
        const std::string apiUrl = "https://api.moonshot.cn/v1/chat/completions";
        std::string response;

        // 设置CURL参数
        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        curl_easy_setopt(curl, CURLOPT_ENCODING, "UTF-8");

        // 设置请求头
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string authHeader = "Authorization: Bearer " + apiKey;
        headers = curl_slist_append(headers, authHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // 构建请求体
        std::string promptsFilePath = config->getApiProviderConfig("kimi")["prompts"].get<std::string>();
        std::vector<std::filesystem::path> searchPaths;
        std::filesystem::path currentPath = std::filesystem::current_path();
        std::filesystem::path configDir = std::filesystem::path(config->getConfigPath()).parent_path();
        
        // 添加搜索路径
        searchPaths.push_back(promptsFilePath);  // 直接路径
        searchPaths.push_back(currentPath / promptsFilePath);  // 当前目录
        searchPaths.push_back(configDir / promptsFilePath);  // 配置文件目录
        
        // 向上查找，直到找到项目根目录（最多查找5层）
        std::filesystem::path tempPath = currentPath;
        for (int i = 0; i < 5 && tempPath.has_parent_path(); ++i) {
            searchPaths.push_back(tempPath / promptsFilePath);
            searchPaths.push_back(tempPath / "config" / "IntentParserPrompt.json");  // 项目标准位置
            tempPath = tempPath.parent_path();
        }
        
        // 尝试所有可能的路径
        std::string triedPaths;
        std::ifstream promptsFile;
        for (const auto& path : searchPaths) {
            if (std::filesystem::exists(path)) {
                promptsFile.open(path);
                if (promptsFile.is_open()) {
                    break;
                }
            }
            triedPaths += "\n              " + path.string();
        }
        
        if (!promptsFile.is_open()) {
            throw std::runtime_error("Failed to open prompts file: " + promptsFilePath + "\n尝试的路径: " + triedPaths);
        }
        std::stringstream buffer;
        buffer << promptsFile.rdbuf();
        auto promptsJson = nlohmann::json::parse(buffer.str());
        nlohmann::json requestBody = {
            {"model", "moonshot-v1-8k"},
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
        if (attempt < maxAttempts) {
            int delay = std::min(initialDelay * (1 << attempt), maxDelay);
            WARNLOG("API call failed, retrying in {} ms (attempt {}/{}): {}", delay, attempt + 1, maxAttempts, e.what());
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            return retryApiCall(query, attempt + 1);
        }
        throw;
    }
}
}

nlohmann::json IntelliSearch::KimiAPIService::processApiResponse(const std::string& response) {
    try {
        DEBUGLOG("Processing API response: {}", response);
        
        if (response.empty()) {
            ERRORLOG("Empty API response received");
            throw std::runtime_error("Empty API response");
        }
    
        // 确保响应是有效的UTF-8编码
        if (!is_valid_utf8(response)) {
            ERRORLOG("Invalid UTF-8 encoding in API response");
            throw std::runtime_error("Invalid UTF-8 encoding in API response");
        }
    
        // 直接解析API响应的JSON
        nlohmann::json jsonResponse = nlohmann::json::parse(response);
        DEBUGLOG("Parsed JSON response: {}", jsonResponse.dump());
        
        // 检查API响应中是否包含错误信息
        if (jsonResponse.contains("error")) {
            std::string errorMessage;
            try {
                errorMessage = jsonResponse["error"]["message"].get<std::string>();
            } catch (const nlohmann::json::exception&) {
                errorMessage = "Unknown API error";
            }
            ERRORLOG("API returned error: {}", errorMessage);
            throw std::runtime_error(errorMessage);
        }
    
        // 提取API响应中的意图理解结果
        if (!jsonResponse.contains("choices") || jsonResponse["choices"].empty()) {
            ERRORLOG("Invalid API response: missing or empty choices");
            throw std::runtime_error("Invalid API response: missing or empty choices");
        }
    
        const auto& firstChoice = jsonResponse["choices"][0];
        if (!firstChoice.contains("message") || !firstChoice["message"].contains("content")) {
            ERRORLOG("API response missing required fields in choices");
            throw std::runtime_error("Invalid API response: missing message content");
        }
    
        // 直接返回完整的JSON响应
        return jsonResponse;
    
    } catch (const nlohmann::json::exception& e) {
        ERRORLOG("Failed to parse API response: {}", e.what());
        throw std::runtime_error(std::string("Failed to parse API response: ") + e.what());
    } catch (const std::exception& e) {
        ERRORLOG("Error processing API response: {}", e.what());
        throw;
    }
}

std::string IntelliSearch::KimiAPIService::utf8_encode(const std::string& str) {
    try {
        // 检查是否已经是有效的UTF-8
        if (is_valid_utf8(str)) {
            return str;
        }
        
        // 如果不是有效的UTF-8，尝试简单的编码转换
        std::string result;
        result.reserve(str.length());
        
        for (unsigned char c : str) {
            if (c < 0x80) { // ASCII字符
                result += c;
            } else { // 非ASCII字符，使用UTF-8编码
                result += (0xC0 | (c >> 6));
                result += (0x80 | (c & 0x3F));
            }
        }
        
        return result;
    } catch (const std::exception& e) {
        ERRORLOG("UTF-8 encoding failed: {}", e.what());
        return str; // 如果转换失败，返回原始字符串
    }
}

bool IntelliSearch::KimiAPIService::is_valid_utf8(const std::string& str) {
    const unsigned char* bytes = (const unsigned char*)str.c_str();
    size_t len = str.length();
    
    for (size_t i = 0; i < len; i++) {
        if (bytes[i] <= 0x7F) { // ASCII字符
            continue;
        } else if (bytes[i] >= 0xC2 && bytes[i] <= 0xDF) { // 2字节序列
            if (i + 1 >= len || (bytes[i + 1] & 0xC0) != 0x80) return false;
            i += 1;
        } else if (bytes[i] >= 0xE0 && bytes[i] <= 0xEF) { // 3字节序列
            if (i + 2 >= len || (bytes[i + 1] & 0xC0) != 0x80 || 
                (bytes[i + 2] & 0xC0) != 0x80) return false;
            i += 2;
        } else if (bytes[i] >= 0xF0 && bytes[i] <= 0xF4) { // 4字节序列
            if (i + 3 >= len || (bytes[i + 1] & 0xC0) != 0x80 || 
                (bytes[i + 2] & 0xC0) != 0x80 || 
                (bytes[i + 3] & 0xC0) != 0x80) return false;
            i += 3;
        } else {
            return false;
        }
    }
    return true;
}