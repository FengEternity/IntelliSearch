#include "BaseAPIService.h"
#include "../../config/ConfigManager.h"
#include "../../log/Logger.h"
#include <fstream>
#include <sstream>

namespace IntelliSearch {

BaseAPIService::BaseAPIService() : curl(nullptr), requestCount(0), lastResetTime(getCurrentTimeMs()) {
    curl = curl_easy_init();
    if (!curl) {
        ERRORLOG("Failed to initialize CURL in BaseAPIService");
        throw std::runtime_error("CURL initialization failed");
    }
}

BaseAPIService::~BaseAPIService() {
    if (curl) {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
}

size_t BaseAPIService::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

int64_t BaseAPIService::getCurrentTimeMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

std::string BaseAPIService::utf8_encode(const std::string& str) {
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

bool BaseAPIService::is_valid_utf8(const std::string& str) {
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

nlohmann::json BaseAPIService::retryApiCall(const std::string& query, int attempt) {
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
        return executeApiCall(query);
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

nlohmann::json BaseAPIService::processApiResponse(const std::string& response) {
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
    
        // 解析API响应的JSON
        nlohmann::json jsonResponse = nlohmann::json::parse(response);
        DEBUGLOG("Parsed JSON response: {}", jsonResponse.dump());
        
        return jsonResponse;
    } catch (const nlohmann::json::exception& e) {
        ERRORLOG("Failed to parse API response: {}", e.what());
        throw std::runtime_error(std::string("Failed to parse API response: ") + e.what());
    } catch (const std::exception& e) {
        ERRORLOG("Error processing API response: {}", e.what());
        throw;
    }
}

void BaseAPIService::setupBasicCurlOptions(const std::string& url, std::string* response) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_ENCODING, "UTF-8");
}

curl_slist* BaseAPIService::setupRequestHeaders(const std::string& contentType, const std::string& authToken) {
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, contentType.c_str());
    if (!authToken.empty()) {
        headers = curl_slist_append(headers, authToken.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    return headers;
}

std::vector<std::filesystem::path> BaseAPIService::buildSearchPaths(const std::string& promptsFilePath) {
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

nlohmann::json BaseAPIService::loadPromptsFile(const std::string& promptsFilePath) {
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

} // namespace IntelliSearch