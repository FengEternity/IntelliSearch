#ifndef INTELLISEARCH_SEARCHSERVICEMANAGER_H
#define INTELLISEARCH_SEARCHSERVICEMANAGER_H

#include "SearchService/SearchService.h"
#include <memory>
#include <vector>
#include <mutex>

namespace IntelliSearch {

class SearchServiceManager {
public:
    static SearchServiceManager* getInstance();

    // 注册新的搜索服务
    void registerService(std::unique_ptr<SearchService> service);

    // 根据服务名称获取服务
    SearchService* getService(const std::string& serviceName);

    // 获取当前可用的最高优先级服务
    SearchService* getPreferredService();

private:
    SearchServiceManager() = default;
    ~SearchServiceManager() = default;

    SearchServiceManager(const SearchServiceManager&) = delete;
    SearchServiceManager& operator=(const SearchServiceManager&) = delete;

    // 选择下一个可用服务（用于故障转移）
    SearchService* selectNextAvailableService();

    static SearchServiceManager* instance;
    static std::mutex instanceMutex;

    std::vector<std::unique_ptr<SearchService>> services;
    std::mutex servicesMutex;
    size_t currentServiceIndex{0};
};

} // namespace IntelliSearch

#endif // INTELLISEARCH_SEARCHSERVICEMANAGER_H 