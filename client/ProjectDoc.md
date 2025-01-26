# IntelliSearch 客户端项目文档

## 项目概述
IntelliSearch客户端是一个基于Qt6框架开发的现代化桌面应用程序，使用C++17标准开发。该客户端提供了直观的用户界面，用于智能搜索功能的交互。

## 项目结构
```
client/
├── CMakeLists.txt      # CMake构建配置文件
├── src/                # 源代码目录
│   ├── main.cpp        # 程序入口文件
│   └── resources/      # 资源文件目录
│       └── resources.qrc  # Qt资源配置文件
```

## 构建系统配置
项目使用CMake作为构建系统，主要配置如下：

### 基础配置
- CMake最低版本要求：3.10
- 项目名称：IntelliSearchClient
- C++标准：C++17
- 启用Qt特性：
  - CMAKE_AUTOMOC：自动处理Qt的Meta-Object系统
  - CMAKE_AUTORCC：自动处理Qt资源文件

### Qt依赖
项目依赖以下Qt6组件：
- Qt6::Core：Qt核心功能
- Qt6::Gui：Qt GUI相关功能
- Qt6::Quick：Qt Quick模块，用于QML界面
- Qt6::QuickControls2：Qt Quick Controls 2，提供现代化UI控件

### 项目文件组织
1. 源文件配置
```cmake
set(SOURCES
    src/main.cpp
)
```

2. 资源文件配置
```cmake
set(RESOURCE_FILES
    src/resources/resources.qrc
)
```

3. 包含目录配置
- 私有包含路径：${CMAKE_SOURCE_DIR}/src

## 开发指南

### 项目构建
1. 创建构建目录：
```bash
mkdir build && cd build
```

2. 配置项目：
```bash
cmake ..
```

3. 编译项目：
```bash
make
```

### 代码规范
- 使用C++17标准特性
- 遵循Qt编码规范
- 使用Qt的信号槽机制进行组件间通信
- QML文件存放在resources目录下

## 技术栈
- 编程语言：C++17
- GUI框架：Qt6
- 构建工具：CMake 3.10+
- UI技术：Qt Quick (QML)
- 版本控制：Git

## 注意事项
1. 确保系统已安装Qt6开发环境
2. 编译时需要注意Qt6的依赖组件是否完整
3. 资源文件的修改需要重新编译项目
4. 开发时注意遵循项目的代码规范和架构设计原则