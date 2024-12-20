cmake_minimum_required(VERSION 3.10)
project(IntellsSearchClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt6 包
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# 自动生成 Qt MOC、UIC 和 RCC 文件
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 指定自动生成的 UIC 搜索路径
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/src)

# 定义源文件、头文件和 UI 文件
set(SOURCES
        src/main.cpp
        src/mainWindow.cpp
)

set(HEADERS
        src/mainWindow.h
)

set(UI
        src/mainWindow.ui
)

# 添加可执行目标
add_executable(IntellsSearchClient
        ${SOURCES}
        ${HEADERS}
        ${UI}
)

# 添加 include 路径
target_include_directories(IntellsSearchClient PRIVATE ${CMAKE_SOURCE_DIR}/src)

# 链接 Qt6 库
target_link_libraries(IntellsSearchClient Qt6::Core Qt6::Gui Qt6::Widgets)

# 显示构建信息
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "CMake Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
