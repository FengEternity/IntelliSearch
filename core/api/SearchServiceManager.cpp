#include "SearchServiceManager.h"
#include "SearchService/Bocha.h"
#include "SearchService/Exa.h"
#include "../log/Logger.h"
#include "../config/ConfigManager.h"

namespace IntelliSearch {

SearchServiceManager* SearchServiceManager::instance = nullptr;
std::mutex SearchServiceManager::instanceMutex;

SearchServiceManager* SearchServiceManager::getInstance() {
    auto* config = ConfigManager::getInstance();
    auto apiProvider = config->getStringValue("search_service", "Bocha");

    static std::map<std::string, std::function<std::unique_ptr<SearchService>()>> serviceMap = {
        {"Bocha", []() {return std::make_unique<Bocha>(); }},
        {"Exa", []() {return std::make_unique<Exa>(); }}
    };

    auto it = serviceMap.find(apiProvider);
    if (it != serviceMap.end()) {
        std::lock_guard<std::mutex> lock(instanceMutex);
        if (instance == nullptr) {
            instance = new SearchServiceManager();
            instance->registerService(it->second());
            INFOLOG("SearchServiceManager initialized with {}", apiProvider);
        }
        return instance;
    } else {
        ERRORLOG("Invalid API provider: {}", apiProvider);
        throw std::runtime_error("Invalid API provider");
    }
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

nlohmann::json SearchServiceManager::performSearch(const std::string& intentResult) {
    std::lock_guard<std::mutex> lock(servicesMutex);

    // 直接实例化具体的搜索对象
    auto* config = ConfigManager::getInstance();
    auto apiProvider = config->getStringValue("search_service", "Bocha");

    if (apiProvider == "Bocha") {
        Bocha bocha;
        return bocha.performSearch(intentResult);
    } else if (apiProvider == "Exa") {
        Exa exa;
        return exa.performSearch(intentResult);
    } else {
        ERRORLOG("Invalid API provider: {}", apiProvider);
        return nlohmann::json{{"error", "Invalid API provider"}};
    }
}

} // namespace IntelliSearch
