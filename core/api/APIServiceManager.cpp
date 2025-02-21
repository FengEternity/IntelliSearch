#include "APIServiceManager.h"
#include "AIService/Kimi.h"
#include "../../log/Logger.h"

namespace IntelliSearch {

APIServiceManager* APIServiceManager::instance = nullptr;
std::mutex APIServiceManager::instanceMutex;

APIServiceManager* APIServiceManager::getInstance() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    if (instance == nullptr) {
        instance = new APIServiceManager();
        // 初始化时注册KimiAPIService
        instance->registerService(std::make_unique<Kimi>());
        INFOLOG("APIServiceManager initialized with Kimi");
    }
    return instance;
}

void APIServiceManager::registerService(std::unique_ptr<APIService> service) {
    std::lock_guard<std::mutex> lock(servicesMutex);
    INFOLOG("Registering API service: {}", service->getServiceName());
    services.push_back(std::move(service));
}

APIService* APIServiceManager::getService(const std::string& serviceName) {
    std::lock_guard<std::mutex> lock(servicesMutex);
    for (const auto& service : services) {
        if (service->getServiceName() == serviceName) {
            return service.get();
        }
    }
    return nullptr;
}

APIService* APIServiceManager::getPreferredService() {
    std::lock_guard<std::mutex> lock(servicesMutex);
    APIService* preferred = nullptr;
    int highestPriority = -1;

    for (const auto& service : services) {
        if (service->isAvailable() && service->getPriority() > highestPriority) {
            preferred = service.get();
            highestPriority = service->getPriority();
        }
    }

    return preferred;
}

APIService* APIServiceManager::selectNextAvailableService() {
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

nlohmann::json APIServiceManager::parseIntent(const std::string& userInput) {
    DEBUGLOG("Parsing intent for input: {}", userInput);
    
    APIService* service = getPreferredService();
    if (!service) {
        service = selectNextAvailableService();
    }

    if (!service) {
        ERRORLOG("No available API service found");
        throw std::runtime_error("No available API service");
    }

    DEBUGLOG("Using service: {} to parse intent", service->getServiceName());
    return service->parseIntent(userInput);
}

} // namespace IntelliSearch