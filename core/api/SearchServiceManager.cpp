#include "SearchServiceManager.h"
#include "SearchService/Bocha.h"
#include "../log/Logger.h"

namespace IntelliSearch {

SearchServiceManager* SearchServiceManager::instance = nullptr;
std::mutex SearchServiceManager::instanceMutex;

SearchServiceManager* SearchServiceManager::getInstance() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    if (instance == nullptr) {
        instance = new SearchServiceManager();
        // 初始化时注册 Bocha 搜索服务
        instance->registerService(std::make_unique<Bocha>());
        INFOLOG("SearchServiceManager initialized with Bocha");
    }
    return instance;
}

void SearchServiceManager::registerService(std::unique_ptr<SearchService> service) {
    std::lock_guard<std::mutex> lock(servicesMutex);
    INFOLOG("Registering Search service: {}", service->getServiceName());
    services.push_back(std::move(service));
}

SearchService* SearchServiceManager::getService(const std::string& serviceName) {
    std::lock_guard<std::mutex> lock(servicesMutex);
    for (const auto& service : services) {
        if (service->getServiceName() == serviceName) {
            return service.get();
        }
    }
    return nullptr;
}

SearchService* SearchServiceManager::getPreferredService() {
    std::lock_guard<std::mutex> lock(servicesMutex);
    SearchService* preferred = nullptr;
    int highestPriority = -1;

    for (const auto& service : services) {
        if (service->isAvailable() && service->getPriority() > highestPriority) {
            preferred = service.get();
            highestPriority = service->getPriority();
        }
    }

    return preferred;
}

SearchService* SearchServiceManager::selectNextAvailableService() {
    std::lock_guard<std::mutex> lock(servicesMutex);
    if (services.empty()) {
        return nullptr;
    }

    size_t startIndex = currentServiceIndex;
    do {
        currentServiceIndex = (currentServiceIndex + 1) % services.size();
        if (services[currentServiceIndex]->isAvailable()) {
            return services[currentServiceIndex].get();
        }
    } while (currentServiceIndex != startIndex);

    return nullptr;
}

} // namespace IntelliSearch 