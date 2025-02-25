#include "AIServiceManager.h"
#include "AIService/Kimi.h"
#include "AIService/Qwen.h"
#include "AIService/Hunyuan.h"
#include "AIService/DeepSeek.h"
#include "../log/Logger.h"
#include "../config/ConfigManager.h"

namespace IntelliSearch {

AIServiceManager* AIServiceManager::instance = nullptr;
std::mutex AIServiceManager::instanceMutex;

AIServiceManager* AIServiceManager::getInstance() {
    auto* config = ConfigManager::getInstance();
    auto apiProvider = config->getStringValue("api_provider", "Kimi");

    static std::map<std::string, std::function<std::unique_ptr<AIService>()>> serviceMap = {
        {"Kimi", []() {return std::make_unique<Kimi>(); }},
        {"Qwen", []() {return std::make_unique<Qwen>(); }},
        {"Hunyuan", []() {return std::make_unique<Hunyuan>(); }},
        {"DeepSeek", []() {return std::make_unique<DeepSeek>(); }}

    };

    auto it = serviceMap.find(apiProvider);
    if (it != serviceMap.end()) {
        std::lock_guard<std::mutex> lock(instanceMutex);
        if (instance == nullptr) {
            instance = new AIServiceManager();
            instance->registerService(it->second());
            INFOLOG("AIServiceManager initialized with {}", apiProvider);
        }
        return instance;
    } else {
        ERRORLOG("Invalid API provider: {}", apiProvider);
        throw std::runtime_error("Invalid API provider");
    }
}

void AIServiceManager::registerService(std::unique_ptr<AIService> service) {
    std::lock_guard<std::mutex> lock(servicesMutex);
    INFOLOG("Registering AI service: {}", service->getServiceName());
    services.push_back(std::move(service));
}

AIService* AIServiceManager::getService(const std::string& serviceName) {
    std::lock_guard<std::mutex> lock(servicesMutex);
    for (const auto& service : services) {
        if (service->getServiceName() == serviceName) {
            return service.get();
        }
    }
    return nullptr;
}

AIService* AIServiceManager::getPreferredService() {
    std::lock_guard<std::mutex> lock(servicesMutex);
    AIService* preferred = nullptr;
    int highestPriority = -1;

    for (const auto& service : services) {
        if (service->isAvailable() && service->getPriority() > highestPriority) {
            preferred = service.get();
            highestPriority = service->getPriority();
        }
    }

    return preferred;
}

AIService* AIServiceManager::selectNextAvailableService() {
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

nlohmann::json AIServiceManager::parseIntent(const std::string& userInput) {
    DEBUGLOG("Parsing intent for input: {}", userInput);
    
    AIService* service = getPreferredService();
    if (!service) {
        service = selectNextAvailableService();
    }

    if (!service) {
        ERRORLOG("No available AI service found");
        throw std::runtime_error("No available AI service");
    }

    DEBUGLOG("Using service: {} to parse intent", service->getServiceName());
    return service->parseIntent(userInput);
}

} // namespace IntelliSearch 