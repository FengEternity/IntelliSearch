#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include "../../config/ConfigManager.h"

namespace IntelliSearch {

class SearchEngine {
public:
    static SearchEngine* getInstance();
    nlohmann::json performSearch(const nlohmann::json& intentResult);
    ~SearchEngine();

private:
    SearchEngine();
    
    nlohmann::json searchWithBocha(const std::string& query, 
                                  const std::string& freshness = "oneYear",
                                  bool summary = true,
                                  int count = 8);
    
    std::string apiKey;
    std::string baseUrl;
    int maxResults;
    int timeoutMs;
    
    void loadConfig();
    static std::unique_ptr<SearchEngine> instance;
};

} // namespace IntelliSearch 