cmake_minimum_required(VERSION 3.10)
project(IntelliSearch)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 设置Homebrew安装的库路径
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/lib/cmake")

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
    Network
    WebEngineCore
    WebEngineWidgets
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
    ${CMAKE_SOURCE_DIR}/../data/crawler/CrawlerManager.cpp
    ${CMAKE_SOURCE_DIR}/../data/crawler/PythonCrawlerBridge.cpp

)

set(RESOURCE_FILES
    qml/resources.qrc
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
    ${CMAKE_SOURCE_DIR}/../data/crawler
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
    Qt6::Network
    Qt6::WebEngineCore
    Qt6::WebEngineWidgets
    CURL::libcurl
    nlohmann_json::nlohmann_json
    Qt6::Sql
)

# 复制配置文件到构建目录
file(COPY ${CMAKE_SOURCE_DIR}/../config/config.json
     DESTINATION ${CMAKE_BINARY_DIR}/config)
file(COPY ${CMAKE_SOURCE_DIR}/../config/IntentParserPrompt.json
     DESTINATION ${CMAKE_BINARY_DIR}/config)
file(COPY ${CMAKE_SOURCE_DIR}/../config/SearchParserPrompt.json
     DESTINATION ${CMAKE_BINARY_DIR}/config)

# 复制Python爬虫相关文件到构建目录
file(COPY ${CMAKE_SOURCE_DIR}/../data/crawler/python_crawler/
     DESTINATION ${CMAKE_BINARY_DIR}/data/crawler/python_crawler
     PATTERN "__pycache__" EXCLUDE
     PATTERN "*.pyc" EXCLUDE
     PATTERN "results" EXCLUDE
     PATTERN "crawl_results" EXCLUDE)

# 显示构建信息
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "CMake Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "CMake Source Dir: ${CMAKE_SOURCE_DIR}")

# 查找Cppcheck并配置静态分析目标
find_program(CPPCHECK_BIN NAMES cppcheck HINTS "/opt/homebrew/bin/")
if(CPPCHECK_BIN)
    set(CPPCHECK_ARGS
        "--library=qt"
        "--suppress=missingIncludeSystem"
        "--suppress=unusedFunction"
        "-I/opt/homebrew/lib/qt6/include/QtCore"
        "-I${CMAKE_SOURCE_DIR}/../log"
        "-I${CMAKE_SOURCE_DIR}/../config"
        "--enable=all"
        "--xml"
        "--xml-version=2"
        "--output-file=${CMAKE_SOURCE_DIR}/cppcheck_report.xml"
        "-i=${CMAKE_BINARY_DIR}"  # 使用 -i 参数代替 --exclude
        "${CMAKE_SOURCE_DIR}/../src"
        "${CMAKE_SOURCE_DIR}/../core"
        "${CMAKE_SOURCE_DIR}/../data"
        "${CMAKE_SOURCE_DIR}/../log"
        "${CMAKE_SOURCE_DIR}/../config"
    )
    add_custom_target(cppcheck
        COMMAND ${CPPCHECK_BIN} ${CPPCHECK_ARGS}
        COMMENT "Running Cppcheck static analysis..."
    )
endif()