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

private:
    // API服务管理器
    class APIServiceManager* apiServiceManager;
};

} // namespace IntelliSearch

#endif // INTELLISEARCH_INTENTPARSER_H