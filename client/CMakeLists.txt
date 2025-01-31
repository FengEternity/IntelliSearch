cmake_minimum_required(VERSION 3.10)
project(IntelliSearchClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 设置Homebrew安装的库路径
# list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/lib/cmake")

# 查找所需的包
find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick QuickControls2)
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

# 定义源文件和资源文件
set(SOURCES
    src/main.cpp
    src/SearchBridge.cpp
    ${CMAKE_SOURCE_DIR}/../core/engine/IntentParser.cpp
    ${CMAKE_SOURCE_DIR}/../core/api/APIServiceManager.cpp
    ${CMAKE_SOURCE_DIR}/../core/api/KimiAPIService.cpp
    ${CMAKE_SOURCE_DIR}/../log/Logger.cpp
    ${CMAKE_SOURCE_DIR}/../config/ConfigManager.cpp
)

set(RESOURCE_FILES
    src/resources/resources.qrc
)

# 添加可执行目标
add_executable(IntellsSearchClient
    ${SOURCES}
    ${RESOURCE_FILES}
)

# 添加 include 路径
target_include_directories(IntellsSearchClient 
    PRIVATE 
    ${CMAKE_SOURCE_DIR}/src 
    ${CMAKE_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/../log
    ${CMAKE_SOURCE_DIR}/../core/engine
    ${CMAKE_SOURCE_DIR}/../core/api
    ${CMAKE_SOURCE_DIR}/../config
    ${CURL_INCLUDE_DIRS}
)

# 链接所有依赖库
target_link_libraries(IntellsSearchClient
    PRIVATE
    fmt::fmt
    spdlog::spdlog
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    CURL::libcurl
    nlohmann_json::nlohmann_json
)

# 显示构建信息
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "CMake Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "CMake Source Dir: ${CMAKE_SOURCE_DIR}")
