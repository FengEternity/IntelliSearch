#ifndef INTELLISEARCH_APISERVICEMANAGER_H
#define INTELLISEARCH_APISERVICEMANAGER_H

#include "AIService/AIService.h"
#include <memory>
#include <vector>
#include <mutex>

namespace IntelliSearch {

class APIServiceManager {
public:
    static APIServiceManager* getInstance();

    // 注册新的 API 服务
    void registerService(std::unique_ptr<APIService> service);

    // 根据服务名称获取服务
    APIService* getService(const std::string& serviceName);

    // 获取当前可用的最高优先级服务
    APIService* getPreferredService();

    // 解析用户输入意图（自动选择合适的服务）
    nlohmann::json parseIntent(const std::string& userInput);

private:
    APIServiceManager() = default;
    ~APIServiceManager() = default;

    // 禁止复制和赋值
    APIServiceManager(const APIServiceManager&) = delete;
    APIServiceManager& operator=(const APIServiceManager&) = delete;

    // 选择下一个可用服务（用于故障转移）
    APIService* selectNextAvailableService();

    static APIServiceManager* instance;
    static std::mutex instanceMutex;

    std::vector<std::unique_ptr<APIService>> services;
    std::mutex servicesMutex;
    size_t currentServiceIndex{0};
};

} // namespace IntelliSearch

#endif // INTELLISEARCH_APISERVICEMANAGER_H