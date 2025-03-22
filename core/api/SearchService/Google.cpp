#include "Google.h"
#include "../../log/Logger.h"
#include "../../config/ConfigManager.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>

namespace IntelliSearch
{

    Google::Google()
    {
        // 从配置文件中读取必要的配置
        auto *config = ConfigManager::getInstance();
        apiKey = config->getApiProviderConfig("google")["api_key"].get<std::string>();
        baseUrl = config->getApiProviderConfig("google")["base_url"].get<std::string>();
        maxResults = config->getApiProviderConfig("google")["max_results"].get<int>();
        timeoutMs = config->getApiProviderConfig("google")["timeout_ms"].get<int>();
        searchEngineId = config->getApiProviderConfig("google")["search_engine_id"].get<std::string>();

        if (apiKey.empty())
        {
            WARNLOG("Bocha API key not found in configuration");
        }

        if (baseUrl.empty())
        {
            WARNLOG("Bocha base URL not found in configuration");
        }

        if (searchEngineId.empty())
        {
            WARNLOG("Bocha search engine ID not found in configuration");
        }

        if (maxResults <= 0)
        {
            WARNLOG("Bocha max results must be greater than 0");
        }

        if (timeoutMs <= 0)
        {
            WARNLOG("Bocha timeout must be greater than 0");
        }
    }

    Google::~Google() = default;


    // 静态回调函数用于接收响应数据
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
    {
        userp->append((char *)contents, size * nmemb);
        return size * nmemb;
    }

    bool Google::validateApiKey() const
    {
        // 简单验证 API 密钥是否存在
        return !apiKey.empty();
    }

    void Google::handleError(const std::string &error)
    {
        ERRORLOG("Google Search API error: " + error);
    }

    nlohmann::json Google::performSearch(const std::string &intentResult)
    {
        try
        {
            // 解析意图结果
            std::string query = intentResult;
            std::string freshness = "";
            bool summary = true;
            int count = 10;

            // 执行搜索
            return search(query, freshness, summary, count);
        }
        catch (const std::exception &e)
        {
            handleError("Failed to parse intent result: " + std::string(e.what()));
            return nlohmann::json();
        }
    }

    nlohmann::json Google::search(
        const std::string &query,
        const std::string &freshness,
        bool summary,
        int count)
    {

        // 构建请求 URL
        std::string url =
            "https://www.googleapis.com/customsearch/v1?key=" + apiKey +
            "&cx=" + searchEngineId +
            "&q=" + curl_easy_escape(nullptr, query.c_str(), query.size()) + // URL 编码查询参数
            "&num=" + std::to_string(std::min(count, maxResults));                            // 返回 10 条结果

        // 如果指定了时间范围
        if (!freshness.empty())
        {
            url += "&dateRestrict=" + freshness;
        }

        // 设置 CURL 选项
        CURL *curl = curl_easy_init();
        if (!curl)
        {
            handleError("Failed to initialize CURL");
            return nlohmann::json();
        }

        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeoutMs);

        // 设置请求头
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // 执行请求
        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK)
        {
            handleError("CURL request failed: " + std::string(curl_easy_strerror(res)));
            return nlohmann::json();
        }

        try
        {
            return nlohmann::json::parse(response);
        }
        catch (const std::exception &e)
        {
            handleError("Failed to parse response: " + std::string(e.what()));
            return nlohmann::json();
        }
    }

    SearchResults Google::processSearchResults(const nlohmann::json &response)
    {
        SearchResults results;

        try
        {
            // 检查响应结构有效性
            if (!response.contains("data") || !response["data"].is_object())
            {
                throw std::runtime_error("Invalid API response structure");
            }

            const auto &data = response["data"];

            // 处理网页结果
            if (data.contains("webPages") && data["webPages"].is_object())
            {
                const auto &webPages = data["webPages"];

                if (webPages.contains("value") && webPages["value"].is_array())
                {
                    for (const auto &item : webPages["value"])
                    {
                        WebPageResult page;
                        if (item.contains("name"))
                            page.title = item["name"].get<std::string>();
                        if (item.contains("url"))
                            page.url = item["url"].get<std::string>();
                        if (item.contains("snippet"))
                            page.snippet = item["snippet"].get<std::string>();
                        if (item.contains("siteName"))
                            page.siteName = item["siteName"].get<std::string>();
                        if (item.contains("dateLastCrawled"))
                            page.date = item["dateLastCrawled"].get<std::string>();
                        if (item.contains("summary"))
                            page.summary = item["summary"].get<std::string>();

                        results.webPages.push_back(page);
                    }
                }

                if (webPages.contains("someResultsRemoved"))
                {
                    results.hasFilteredResults = webPages["someResultsRemoved"].get<bool>();
                }
            }

            // 处理图片结果
            if (data.contains("images") && data["images"].is_object())
            {
                const auto &images = data["images"];

                if (images.contains("value") && images["value"].is_array())
                {
                    for (const auto &item : images["value"])
                    {
                        ImageResult img;
                        if (item.contains("thumbnailUrl"))
                            img.thumbnailUrl = item["thumbnailUrl"].get<std::string>();
                        if (item.contains("contentUrl"))
                            img.contentUrl = item["contentUrl"].get<std::string>();

                        results.images.push_back(img);
                    }
                }
            }

            // 记录处理结果摘要
            INFOLOG("Processed {} web results and {} image results",
                    results.webPages.size(),
                    results.images.size());

            // 详细日志调试模式
            DEBUGLOG("Search Results Details:\n{}\n{}", [&]()
                     {
                    std::stringstream ss;
                    ss << "\n=== Web Results ===";
                    for (size_t i = 0; i < results.webPages.size(); ++i) {
                        const auto& page = results.webPages[i];
                        ss << "\n[" << i+1 << "]"
                           << "\n    Title: " << (page.title.empty() ? "[No Title]" : page.title)
                           << "\n    URL: " << (page.url.empty() ? "[No URL]" : page.url)
                           << "\n    Snippet: " << (page.snippet.empty() ? "[No Snippet]" : page.snippet)
                           << "\n    Site Name: " << (page.siteName.empty() ? "[No Site Name]" : page.siteName)
                           << "\n    Summary: " << (page.summary.empty() ? "[No Summary]" : page.summary)
                           << "\n    Date: " << (page.date.empty() ? "[No Date]" : page.date)
                           << "\n";
                    }
                    return ss.str(); }(), [&]()
                     {
                    std::stringstream ss;
                    ss << "\n=== Image Results ===";
                    for (size_t i = 0; i < results.images.size(); ++i) {
                        const auto& img = results.images[i];
                        ss << "\n[" << i+1 << "]"
                           << "\n    Thumbnail URL: " << (img.thumbnailUrl.empty() ? "[No Thumbnail]" : img.thumbnailUrl)
                           << "\n    Content URL: " << (img.contentUrl.empty() ? "[No Content URL]" : img.contentUrl)
                           << "\n";
                    }
                    return ss.str(); }());

            // 记录过滤状态
            if (results.hasFilteredResults)
            {
                WARNLOG("Partial results filtered by search provider");
            }

            return results;
        }
        catch (const std::exception &e)
        {
            ERRORLOG("Result processing failed: {}", e.what());
            throw;
        }
    }

} // namespace IntelliSearch