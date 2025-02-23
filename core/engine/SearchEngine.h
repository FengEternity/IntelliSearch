#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include "../api/SearchServiceManager.h"

namespace IntelliSearch {

class SearchEngine {
public:
    static SearchEngine* getInstance();
    nlohmann::json performSearch(const std::string& intentResult);
    ~SearchEngine();

private:
    SearchEngine();
    
    SearchServiceManager* serviceManager;
    
    static std::unique_ptr<SearchEngine> instance;
};

} // namespace IntelliSearch 