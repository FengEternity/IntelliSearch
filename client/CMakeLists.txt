cmake_minimum_required(VERSION 3.10)
project(IntelliSearch)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 设置Homebrew安装的库路径
# list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/lib/cmake")

# 查找所需的包
find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Gui 
    Quick 
    QuickControls2 
    Sql
    Concurrent
    Widgets
    QuickTemplates2
)
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

# 定义源文件和资源文件
set(SOURCES
    src/main.cpp
    src/SearchBridge.cpp
    ${CMAKE_SOURCE_DIR}/../core/engine/IntentParser.cpp
    ${CMAKE_SOURCE_DIR}/../core/engine/SearchEngine.cpp

    ${CMAKE_SOURCE_DIR}/../core/api/AIServiceManager.cpp
    ${CMAKE_SOURCE_DIR}/../core/api/SearchServiceManager.cpp

    ${CMAKE_SOURCE_DIR}/../core/api/AIService/AIService.cpp
    ${CMAKE_SOURCE_DIR}/../core/api/AIService/Kimi.cpp
    ${CMAKE_SOURCE_DIR}/../core/api/AIService/Qwen.cpp
    ${CMAKE_SOURCE_DIR}/../core/api/AIService/Hunyuan.cpp
    ${CMAKE_SOURCE_DIR}/../core/api/AIService/DeepSeek.cpp

    ${CMAKE_SOURCE_DIR}/../core/api/SearchService/SearchService.cpp
    ${CMAKE_SOURCE_DIR}/../core/api/SearchService/Bocha.cpp
    ${CMAKE_SOURCE_DIR}/../core/api/SearchService/Exa.cpp

    ${CMAKE_SOURCE_DIR}/../log/Logger.cpp
    ${CMAKE_SOURCE_DIR}/../config/ConfigManager.cpp
    ${CMAKE_SOURCE_DIR}/../data/database/DatabaseManager.cpp
)

set(RESOURCE_FILES
    src/resources/resources.qrc
)

# 添加可执行目标
add_executable(IntelliSearch
    ${SOURCES}
    ${RESOURCE_FILES}
)

# 添加 include 路径
target_include_directories(IntelliSearch 
    PRIVATE 
    ${CMAKE_SOURCE_DIR}/src 
    ${CMAKE_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/../log
    ${CMAKE_SOURCE_DIR}/../core/engine
    ${CMAKE_SOURCE_DIR}/../core/api
    ${CMAKE_SOURCE_DIR}/../config
    ${CMAKE_SOURCE_DIR}/../data/database
    ${CURL_INCLUDE_DIRS}
)

# 链接所有依赖库
target_link_libraries(IntelliSearch
    PRIVATE
    fmt::fmt
    spdlog::spdlog
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Concurrent
    Qt6::Widgets
    Qt6::QuickTemplates2
    CURL::libcurl
    nlohmann_json::nlohmann_json
    Qt6::Sql
)

# 复制配置文件到构建目录
file(COPY ${CMAKE_SOURCE_DIR}/../config/config.json
     DESTINATION ${CMAKE_BINARY_DIR}/config)
file(COPY ${CMAKE_SOURCE_DIR}/../config/IntentParserPrompt.json
     DESTINATION ${CMAKE_BINARY_DIR}/config)
file(COPY ${CMAKE_SOURCE_DIR}/../config/intent_rules.json
     DESTINATION ${CMAKE_BINARY_DIR}/config)

# 显示构建信息
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "CMake Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "CMake Source Dir: ${CMAKE_SOURCE_DIR}")
