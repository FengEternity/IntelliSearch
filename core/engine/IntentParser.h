#ifndef INTELLISEARCH_INTENTPARSER_H
#define INTELLISEARCH_INTENTPARSER_H

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace IntelliSearch {

class IntentParser {
public:
    IntentParser();
    ~IntentParser();

    // 从搜索栏获取用户输入并解析意图
    nlohmann::json parseSearchIntent(const std::string& userInput);

    // 调用博查API进行搜索
    nlohmann::json bochaSearch(const std::string& query);

private:
    // 本地意图解析
    nlohmann::json localIntentParsing(const std::string& input);
    
    // 混合意图解析结果
    nlohmann::json mergeIntentResults(const nlohmann::json& localResult, const nlohmann::json& apiResult);
    
    // 加载意图解析配置
    void loadIntentConfig();

    // API服务管理器
    class APIServiceManager* apiServiceManager;
    
    // 意图解析配置
    nlohmann::json intentConfig;
};

} // namespace IntelliSearch

#endif // INTELLISEARCH_INTENTPARSER_H