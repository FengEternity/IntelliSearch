# spdlog 安装和使用指南

## 安装方法

### 方法一：使用包管理器

```bash
# Ubuntu/Debian
sudo apt-get install libspdlog-dev

# macOS
brew install spdlog

# Windows (使用 vcpkg)
vcpkg install spdlog
```

### 方法二：从源码编译

```bash
git clone https://github.com/gabime/spdlog.git
cd spdlog && mkdir build && cd build
cmake .. && make -j
sudo make install
```

## CMake 配置

在 CMakeLists.txt 中添加：

```cmake
find_package(spdlog REQUIRED)
target_link_libraries(your_target PRIVATE spdlog::spdlog)
```


### 编译问题

在编译的过程中遇到 fmt 库链接失败的问题，最终发现是因为 fmt 是由 conda 安装，但是   spdlog 是由 homebrew 安装导致的冲突。

## 基本使用

### 1. 日志级别

spdlog 提供以下日志级别（从低到高）：

- trace
- debug
- info
- warn
- error
- critical

### 2. 基本用法示例

```cpp
#include "spdlog/spdlog.h"

int main() {
    // 设置全局日志级别
    spdlog::set_level(spdlog::level::debug);

    // 基本日志输出
    spdlog::info("Welcome to spdlog!");
    spdlog::error("Some error message with arg: {}", 1);
    spdlog::warn("Easy padding in numbers like {:08d}", 12);
    spdlog::critical("Support for int: {0:d}; hex: {0:x}; oct: {0:o}; bin: {0:b}", 42);

    // 使用 fmt 库的格式化功能
    spdlog::info("Support for floats {:03.2f}", 1.23456);
    spdlog::info("Positional args are {1} {0}..", "too", "supported");
    spdlog::info("{:<30}", "left aligned");
}
```

### 3. 文件日志记录器

```cpp
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"

int main() {
    // 基本文件日志
    auto logger = spdlog::basic_logger_mt("basic_logger", "logs/basic.log");

    // 循环文件日志（最大文件大小为 1MB，保留 3 个历史文件）
    auto rotating_logger = spdlog::rotating_logger_mt("rotating_logger", "logs/rotating.log", 1024*1024, 3);

    // 使用特定的日志记录器
    rotating_logger->info("This will be logged to rotating file");
    rotating_logger->flush(); // 立即写入文件
}
```

### 4. 异步日志

```cpp
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"

int main() {
    // 创建异步日志记录器
    auto async_file = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", "logs/async.log");

    // 设置异步队列大小（可选）
    spdlog::init_thread_pool(8192, 1);

    async_file->info("Async message");
}
```

## 在当前项目中的使用

当前项目已经集成了 spdlog，并封装了一个 Logger 类（在 log/Logger.h 和 log/Logger.cpp 中）。使用方法如下：

```cpp
#include "log/Logger.h"

int main() {
    // 配置日志
    LogConfig config;
    config.level = "debug";
    config.path = "logs/app.log";
    config.size = 1024 * 1024; // 1MB
    config.count = 3;

    // 初始化日志
    INITLOG(config);

    // 使用日志宏
    TRACELOG("这是一条 trace 日志");
    DEBUGLOG("这是一条 debug 日志");
    INFOLOG("这是一条 info 日志，参数: {}", 42);
    WARNLOG("这是一条 warning 日志");
    ERRORLOG("这是一条 error 日志");
    CRITICALLOG("这是一条 critical 日志");

    return 0;
}
```

## 注意事项

1. 日志文件路径要确保目录存在，否则需要提前创建
2. 合理设置日志级别，生产环境通常设置为 info 或更高级别
3. 定期清理或归档日志文件，避免占用过多磁盘空间
4. 在程序退出前确保日志已经完全写入（flush）
