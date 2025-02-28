#include "AIService.h"
#include "../../../log/Logger.h"
#include "../../../config/ConfigManager.h"
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <thread>
#include <chrono>

namespace IntelliSearch {

/*
 * Summary: AIService构造函数
 * Parameters:
 *   QObject* parent - 父对象指针
 * Description: 初始化AIService，设置CURL句柄，并检查初始化是否成功
 */
AIService::AIService() : curl(nullptr), requestCount(0), lastResetTime(getCurrentTimeMs()) {
    curl = curl_easy_init();
    if (!curl) {
        ERRORLOG("Failed to initialize CURL in AIService");
        throw std::runtime_error("CURL initialization failed");
    }
}
/*
* Summary: AIService析构函数
* Description: 释放CURL句柄
*/
AIService::~AIService() {
    if (curl) {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
}

/*
* Summary: CURL写回调函数
* Parameters:
*   void* contents - 回调数据
*   size_t size - 数据大小
*   size_t nmemb - 数据数量
*   std::string* userp - 用户数据指针
*/
size_t AIService::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

/*
* Summary: 获取当前时间戳（毫秒）
* Returns:
*   int64_t - 当前时间戳（毫秒）
*/
int64_t AIService::getCurrentTimeMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

/*
* Summary: 将字符串编码为UTF-8
* Parameters:
*   const std::string& str - 需要编码的字符串
* Returns:
*   std::string - 编码后的字符串
*/
std::string AIService::utf8_encode(const std::string& str) {
    try {
        if (is_valid_utf8(str)) {
            return str;
        }
        
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
        return str;
    }
}

/*
* Summary: 检查字符串是否为UTF-8编码
* Parameters:
*   const std::string& str - 需要检查的字符串
* Returns:
*   bool - 如果字符串为UTF-8编码，返回true，否则返回false
*/
bool AIService::is_valid_utf8(const std::string& str) {
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

/*
* Summary: 重试API调用
* Parameters:
*   const std::string& query - API查询字符串
*   int attempt - 当前重试次数
* Returns:
*   nlohmann::json - API响应
*/
nlohmann::json AIService::retryApiCall(const std::string& query, const std::string& promptType, int attempt) {
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
        return executeApiCall(query, promptType);
    } catch (const std::exception& e) {
        if (attempt < maxAttempts) {
            int delay = std::min(initialDelay * (1 << attempt), maxDelay);
            WARNLOG("API call failed, retrying in {} ms (attempt {}/{}): {}", delay, attempt + 1, maxAttempts, e.what());
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            return retryApiCall(query, promptType, attempt + 1);
        }
        throw;
    }
}

/*
* Summary: 设置基本的CURL选项
* Parameters:
*   const std::string& url - API URL
*   std::string* response - 存储API响应的指针
*/
void AIService::setupBasicCurlOptions(const std::string& url, std::string* response) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_ENCODING, "UTF-8");
}

/*
* Summary: 设置请求头
* Parameters:
*   const std::string& contentType - 内容类型
*   const std::string& authToken - 认证令牌
* Returns:
*   curl_slist* - 请求头列表
*/
curl_slist* AIService::setupRequestHeaders(const std::string& contentType, const std::string& authToken) {
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, contentType.c_str());
    if (!authToken.empty()) {
        headers = curl_slist_append(headers, authToken.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    return headers;
}

/*
* Summary: 查找并读取提示文件
* Parameters:
*   const std::string& promptsFilePath - 提示文件路径
* Returns:
*   nlohmann::json - 提示文件内容
*/
nlohmann::json AIService::loadPromptsFile(const std::string& promptsFilePath) {
    auto searchPaths = buildSearchPaths(promptsFilePath);
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
    return nlohmann::json::parse(buffer.str());
}

/*
* Summary: 构建搜索路径列表
* Parameters:
*   const std::string& promptsFilePath - 提示文件路径
* Returns:
*   std::vector<std::filesystem::path> - 搜索路径列表
*/
std::vector<std::filesystem::path> AIService::buildSearchPaths(const std::string& promptsFilePath) {
    std::vector<std::filesystem::path> searchPaths;
    auto* config = ConfigManager::getInstance();
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path configDir = std::filesystem::path(config->getConfigPath()).parent_path();
    
    searchPaths.push_back(promptsFilePath);
    searchPaths.push_back(currentPath / promptsFilePath);
    searchPaths.push_back(configDir / promptsFilePath);
    
    std::filesystem::path tempPath = currentPath;
    for (int i = 0; i < 5 && tempPath.has_parent_path(); ++i) {
        searchPaths.push_back(tempPath / promptsFilePath);
        searchPaths.push_back(tempPath / "config" / "IntentParserPrompt.json");
        tempPath = tempPath.parent_path();
    }
    
    return searchPaths;
}

    /*
    * Summary: 处理API响应
    * Parameters:
    *   const std::string& response - API响应
    * Returns:
    *   nlohmann::json - 处理后的API响应
    */
    nlohmann::json AIService::processApiResponse(const std::string& response) {
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