#ifndef INTELLISEARCH_BOCHA_H
#define INTELLISEARCH_BOCHA_H

#include "SearchService.h"

namespace IntelliSearch {

class Bocha : public SearchService {
public:
    Bocha();
    ~Bocha() override;

    // 实现 APIService 的纯虚函数
    std::string getServiceName() const override { return "Bocha"; }
    bool isAvailable() const override { return !apiKey.empty() && validateApiKey(); }
    int getPriority() const override { return 1; }
    void handleError(const std::string& error) override;
    bool validateApiKey() const override;

    // 实现 SearchService 的纯虚函数
    nlohmann::json performSearch(const std::string& intentResult) override;
    nlohmann::json search(
         const std::string& query, // 用户的搜索词
         const std::string& freshness, // 搜索指定时间范围内的网页
         bool summary, // 是否显示文本摘要
         int count // 返回结果的条数
         );

    SearchResults processSearchResults(const nlohmann::json& response);

private:
    std::string apiKey; // API 密钥
    std::string baseUrl; // 基础 URL
    int maxResults; // 最大结果数
    int timeoutMs; // 超时时间
};

} // namespace IntelliSearch

#endif // INTELLISEARCH_BOCHA_H