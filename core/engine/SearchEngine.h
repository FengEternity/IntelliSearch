#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include "../api/SearchServiceManager.h"
#include "../api/AIServiceManager.h"

namespace IntelliSearch {

class SearchEngine {
public:
    static SearchEngine* getInstance();
    nlohmann::json performSearch(const std::string& intentResult);
    nlohmann::json analyzeSearchResults(const nlohmann::json& searchResults, const std::string& userQuery);
    ~SearchEngine();

private:
    SearchEngine();
    
    SearchServiceManager* searchServiceManager;
    AIServiceManager* aiServiceManager;
    
    static std::unique_ptr<SearchEngine> instance;
};

} // namespace IntelliSearch 