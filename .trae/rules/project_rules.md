# IntelliSearch 项目贡献指南

欢迎为 IntelliSearch 项目做出贡献！为了确保项目代码的质量和开发流程的顺畅，请遵循以下规则。

## 目录

1.  [代码风格](#代码风格)
2.  [Commit 提交规范](#commit-提交规范)
3.  [分支管理策略](#分支管理策略)
4.  [构建系统](#构建系统)
5.  [目录结构](#目录结构)
6.  [依赖管理](#依赖管理)
7.  [测试要求](#测试要求)
8.  [文档规范](#文档规范)
9.  [代码审查](#代码审查)
10. [问题跟踪](#问题跟踪)
11. [IDE 集成](#ide-集成)

## 代码风格

*   **C++:**
    *   遵循 C++17 标准。
    *   遵循 [Qt 编码风格指南](https://wiki.qt.io/Qt_Coding_Style)。
    *   使用项目内 <mcfile name="Logger.h" path="/Users/montylee/Library/Mobile Documents/com~apple~CloudDocs/Forsertee/毕业设计/Code/IntelliSearch/log/Logger.h"></mcfile> 进行日志记录。
    *   头文件包含顺序：系统头文件 -> Qt 头文件 -> 第三方库头文件 -> 项目内头文件。
    *   代码格式化：推荐使用 `clang-format`，配置文件待定。
    *   遵循 <mcfile name="CodeStandard.md" path="/Users/montylee/Library/Mobile Documents/com~apple~CloudDocs/Forsertee/毕业设计/Code/IntelliSearch/docs/CodeStandard.md"></mcfile> 中关于文件头注释和函数注释的规范。
    *   行长度限制为 120 个字符。
    *   缩进使用 4 个空格。
    *   遵循以下命名约定：
        *   类和命名空间：`PascalCase`
        *   函数和局部变量：`camelCase`
        *   成员变量：`m_camelCase`
        *   静态变量：`s_camelCase`
        *   常量：`UPPER_CASE`
*   **Python (爬虫部分):**
    *   遵循 PEP 8 风格指南。
    *   添加必要的类型提示 (Type Hinting)。
    *   使用 Python 内置 `logging` 模块进行日志记录。
*   **QML:**
    *   遵循 Qt 官方 QML 编码规范。
    *   组件化开发，复用 UI 元素。
    *   缩进使用 4 个空格。
    *   行长度限制为 100 个字符。
    *   `id` 和属性命名使用 `camelCase`。

## Commit 提交规范

*   严格遵循 <mcfile name="CodeStandard.md" path="/Users/montylee/Library/Mobile Documents/com~apple~CloudDocs/Forsertee/毕业设计/Code/IntelliSearch/docs/CodeStandard.md"></mcfile> 中定义的 Commit Message 格式。
*   Commit Message 类型包括：`feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`。
*   每次提交应尽可能原子化，只包含一个逻辑变更。
*   Commit Message 使用中文编写。

## 分支管理策略

*   采用 **Gitflow** 或类似的分支模型：
    *   `main` (或 `master`): 稳定发布分支。
    *   `develop`: 主要开发分支，汇集所有已完成的功能。
    *   `feature/xxx`: 功能开发分支，从 `develop` 分支创建，完成后合并回 `develop`。
    *   `fix/xxx`: Bug 修复分支，从 `develop` 或 `main` 创建，修复后合并回对应分支。
    *   `release/xxx`: 发布准备分支，从 `develop` 创建，用于发布前的测试和微调。
*   禁止直接向 `main` 和 `develop` 分支推送代码，所有更改必须通过 Pull Request (PR) 合并。

## 构建系统

*   使用 CMake (最低版本 3.10) 进行项目构建。
*   支持的构建类型：`Debug`, `Release`。
*   构建输出目录：`build`。
*   Qt 最低版本要求：6.0。
*   必需的 Qt 模块：`Core`, `Gui`, `Quick`, `QuickControls2`, `Sql`, `Concurrent`, `Network`。

## 目录结构

*   **源代码目录:**
    *   `client/src`
    *   `client/qml`
    *   `core/api`
    *   `core/engine`
    *   `data/crawler`
    *   `data/database`
*   **资源目录:**
    *   `client/qml/resources`
    *   `config`
    *   `docs` (除 API 文档外的其他文档)
    *   `image`
*   **测试目录:** (需要创建)
    *   `tests/unit`
    *   `tests/integration`
*   **排除目录:** (不应提交到版本控制)
    *   `build`
    *   `log` (运行时日志文件)
    *   `.git`

## 依赖管理

*   **C++:** 通过 CMake (<mcfile name="CMakeLists.txt" path="/Users/montylee/Library/Mobile Documents/com~apple~CloudDocs/Forsertee/毕业设计/Code/IntelliSearch/client/CMakeLists.txt"></mcfile>) 管理依赖。添加新依赖需在 CMake 文件中明确声明，并说明原因。
    *   **必需依赖:**
        *   `spdlog (^1.9.0)`
        *   `nlohmann_json (^3.9.0)`
        *   `curl (^7.0.0)`
        *   `sqlite3 (^3.0.0)`
    *   **可选依赖:**
        *   `python3 (^3.8.0)` (用于爬虫模块)
*   **Python:** 使用 `requirements.txt` 文件管理 Python 依赖（目前缺失，建议在 <mcfolder name="python_crawler" path="/Users/montylee/Library/Mobile Documents/com~apple~CloudDocs/Forsertee/毕业设计/Code/IntelliSearch/data/crawler/python_crawler"></mcfolder> 目录下创建）。

## 测试要求

*   **单元测试:** 核心模块（如 <mcfolder name="core" path="/Users/montylee/Library/Mobile Documents/com~apple~CloudDocs/Forsertee/毕业设计/Code/IntelliSearch/core"></mcfolder> 目录下的逻辑）应编写单元测试。
*   **集成测试:** 对涉及多个模块交互的功能（如搜索流程、爬虫与数据处理）编写集成测试。
*   **客户端测试:** 对 UI 交互和功能进行手动测试或使用自动化 UI 测试框架。
*   所有新功能和 Bug 修复都应伴随相应的测试用例。
*   测试代码应放置在 `tests/` 目录下（目前该目录为空，需要创建）。
*   **测试框架:** 使用 `GoogleTest` 和 `QtTest`。
*   **代码覆盖率:** 目标覆盖率不低于 80%。

## 文档规范

*   **代码注释:**
    *   遵循 <mcfile name="CodeStandard.md" path="/Users/montylee/Library/Mobile Documents/com~apple~CloudDocs/Forsertee/毕业设计/Code/IntelliSearch/docs/CodeStandard.md"></mcfile> 中的文件头和函数注释规范。
    *   注释风格遵循 `Doxygen`。
    *   函数/方法注释应包含以下部分（根据需要）：`@brief`, `@param`, `@return`, `@throws`, `@thread_safety`。
    *   对复杂逻辑添加必要的行内注释。
*   **接口文档:** API 接口（如 <mcfile name="AIService.h" path="/Users/montylee/Library/Mobile Documents/com~apple~CloudDocs/Forsertee/毕业设计/Code/IntelliSearch/core/api/AIService/AIService.h"></mcfile>, <mcfile name="SearchBridge.h" path="/Users/montylee/Library/Mobile Documents/com~apple~CloudDocs/Forsertee/毕业设计/Code/IntelliSearch/client/src/SearchBridge.h"></mcfile>）应有清晰的文档说明，建议使用 Doxygen 生成。
*   **API 文档输出:** 生成的 API 文档应放置在 `docs/api` 目录下。
*   **设计文档:** 重大功能或架构变更需要更新或创建相应的设计文档（存放于 <mcfolder name="docs" path="/Users/montylee/Library/Mobile Documents/com~apple~CloudDocs/Forsertee/毕业设计/Code/IntelliSearch/docs"></mcfolder> 目录）。
*   **README:** 保持 <mcfile name="README.md" path="/Users/montylee/Library/Mobile Documents/com~apple~CloudDocs/Forsertee/毕业设计/Code/IntelliSearch/README.md"></mcfile> 文件更新，包含项目简介、安装、运行方法等。

## 代码审查

*   所有向 `develop` 或 `main` 分支合并的 Pull Request (PR) 必须经过至少一位其他开发者的审查 (Review)。
*   审查者关注点：代码风格、逻辑正确性、测试覆盖率、文档完整性、是否符合项目规范。
*   PR 作者需要根据审查意见进行修改，直至通过审查。

## 问题跟踪

*   使用 GitHub Issues (或其他选定的项目管理工具) 跟踪 Bug、功能请求和任务。
*   创建 Issue 时，请提供清晰的描述、复现步骤（如果是 Bug）、预期行为等。

## IDE 集成

*   推荐配置 IDE (如 VS Code, CLion, Cursor) 以符合项目规范：
    *   **保存时自动格式化:** 使用 `clang-format` (配置文件待定)。
    *   **智能提示:** 启用 C++, Qt, QML, Python 的智能提示和补全。
    *   **代码分析:** 启用静态代码分析工具 (如 Clang-Tidy, Cppcheck)。
    *   **换行符:** 统一使用 `LF` 作为换行符。
