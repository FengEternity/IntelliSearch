#include "SearchService.h"
#include "../../../log/Logger.h"
#include "../../../config/ConfigManager.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>
#include <stdexcept>

namespace IntelliSearch {

// 静态回调函数用于接收响应数据
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/*
 * Summary: SearchService构造函数
 * Parameters:
 *   QObject* parent - 父对象指针
 * Description: 初始化SearchService，设置CURL句柄，并检查初始化是否成功
 */
SearchService::SearchService() {
    curl = curl_easy_init();
    if (!curl) {
        ERRORLOG("Failed to initialize CURL");
        throw std::runtime_error("Failed to initialize CURL");
    }
    requestCount = 0;
    lastResetTime = getCurrentTimeMs();
}

/*
 * Summary: SearchService析构函数
 * Description: 释放CURL句柄
 */
SearchService::~SearchService() {
    // 释放搜索服务资源
    if (curl) {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
}

void SearchService::setupBasicCurlOptions(const std::string& url, std::string* response) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
}

struct curl_slist* SearchService::setupRequestHeaders(const std::string& contentType, const std::string& authHeader) {
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, contentType.c_str());
    headers = curl_slist_append(headers, authHeader.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    return headers;
}

} // namespace IntelliSearch