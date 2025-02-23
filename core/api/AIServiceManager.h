#ifndef INTELLISEARCH_AISERVICEMANAGER_H
#define INTELLISEARCH_AISERVICEMANAGER_H

#include "AIService/AIService.h"
#include <memory>
#include <vector>
#include <mutex>

namespace IntelliSearch {

class AIServiceManager {
public:
    static AIServiceManager* getInstance();

    // 注册新的 AI 服务
    void registerService(std::unique_ptr<AIService> service);

    // 根据服务名称获取服务
    AIService* getService(const std::string& serviceName);

    // 获取当前可用的最高优先级服务
    AIService* getPreferredService();

    // 解析用户输入意图
    nlohmann::json parseIntent(const std::string& userInput);

private:
    AIServiceManager() = default;
    ~AIServiceManager() = default;

    AIServiceManager(const AIServiceManager&) = delete;
    AIServiceManager& operator=(const AIServiceManager&) = delete;

    // 选择下一个可用服务（用于故障转移）
    AIService* selectNextAvailableService();

    static AIServiceManager* instance;
    static std::mutex instanceMutex;

    std::vector<std::unique_ptr<AIService>> services;
    std::mutex servicesMutex;
    size_t currentServiceIndex{0};
};

} // namespace IntelliSearch

#endif // INTELLISEARCH_AISERVICEMANAGER_H 