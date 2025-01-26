cmake_minimum_required(VERSION 3.10)
project(IntelliSearchClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 查找 Qt6 包
find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick QuickControls2)

# 定义源文件和资源文件
set(SOURCES
    src/main.cpp
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
target_include_directories(IntellsSearchClient PRIVATE ${CMAKE_SOURCE_DIR}/src)

# 链接 Qt6 库
target_link_libraries(IntellsSearchClient
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
)

# 显示构建信息
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "CMake Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
