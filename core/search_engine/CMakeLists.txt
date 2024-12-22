cmake_minimum_required(VERSION 3.10)

# 设置新的CMake政策以解决FindBoost警告
cmake_policy(SET CMP0167 NEW)

project(IntelliSearchEngine)

# 使用 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置 vcpkg 工具链文件
set(CMAKE_TOOLCHAIN_FILE "/opt/vcpkg/scripts/buildsystems/vcpkg.cmake")

# 找到 spdlog
find_package(spdlog CONFIG REQUIRED)

# 找到系统安装的 Boost，使用 CONFIG 模式
find_package(Boost 1.71 CONFIG REQUIRED COMPONENTS filesystem system)

if (Boost_FOUND)
    add_executable(IntelliSearchEngine
            src/main.cpp
            ../../log/Logger.h
            ../../log/Logger.cpp
    )

    target_include_directories(IntelliSearchEngine PRIVATE
            ${Boost_INCLUDE_DIRS}
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/../../log
    )

    target_link_libraries(IntelliSearchEngine PRIVATE
            Boost::filesystem
            Boost::system
            spdlog::spdlog
    )
else ()
    message(FATAL_ERROR "Boost not found!")
endif ()