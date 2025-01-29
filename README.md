# IntelliSearch：基于AI大语言模型的对话式搜索引擎

## 项目简介

本项目是一个基于AI大语言模型的对话式搜索引擎，旨在通过自然语言交互提升用户的信息检索体验。项目结合了大语言模型、搜索引擎和数据管理技术，开发了一个支持跨平台的智能搜索系统。

## 核心功能

* 对话式搜索：支持用户以自然语言输入查询，提供智能检索结果。
* 多模态交互：支持文本和图像搜索。
* 数据管理：高效的数据存储和检索机制，保障数据隐私。
* 跨平台客户端：基于Qt开发的用户友好界面，支持Windows、Linux、macOS。

## 项目特点

1. 模型支持：

* 集成主流大语言模型（如GPT-4、星火、通义千问）。
* 本地化部署，保护用户数据隐私。

2. 搜索优化：

* 结合语义搜索与传统索引，提高检索效率。
* 使用ElasticSearch进行倒排索引管理。

3. 高效架构：

* 模块化设计，易于扩展和维护。
* 使用Docker容器化部署，支持多环境运行。

## 项目结构

```plaintext
├── client/                   # 客户端代码
│   ├── main.cpp              # 主入口文件
│   ├── ui/                   # 界面设计
│   └── resources/            # 客户端资源
├── core/                     # 搜索核心模块
│   ├── model/                # 模型相关代码
│   ├── search_engine/        # 搜索引擎实现
│   └── api/                  # API接口
├── data/                     # 数据管理模块
│   ├── database/             # 数据库脚本
│   ├── cache/                # 缓存管理
│   └── crawler/              # 数据爬取工具
├── docs/                     # 项目文档
│   ├── user_manual.md        # 用户手册
│   └── developer_guide.md    # 开发者指南
├── tests/                    # 测试模块
│   ├── unit_tests/           # 单元测试
│   └── integration_tests/    # 集成测试
├── Dockerfile                # Docker配置文件
├── README.md                 # 项目简介
└── LICENSE                   # 开源协议
```

## 技术栈

* 编程语言：C++、Python
* 前端框架：Qt
* 搜索引擎：ElasticSearch
* 模型框架：Ollama
* 数据库：MySQL、Redis
* 容器化：Docker

## 安装与运行

1. 环境准备

   * 操作系统：支持Windows、Linux、macOS
   * 必备工具：

     * Docker
     * C++编译器（支持C++17及以上）
     * Qt框架
     * Python 3.8+
2. 克隆项目

   ```shell
   git clone https://github.com/your_username/IntelliSearch.git
   cd IntelliSearch
   ```
3. 安装依赖

使用Docker运行：

```shell
docker build -t ai-search-engine .
docker run -p 8080:8080 ai-search-engine
```

本地运行：

1. 安装 Qt 框架和必要依赖。
2. 配置数据库和 ElasticSearch。
3. 编译并运行客户端：

```shell
cd client/
mkdir build && cd build
cmake .. && make
./IntelliSearch
```

## 使用说明

1. 用户功能

   * 启动应用后，在搜索框输入自然语言查询，点击“搜索”按钮。
   * 通过对话界面查看推荐结果并进行进一步查询。
2. 开发者指南

   * 修改大语言模型接口：编辑 core/api/model_interface.cpp。
   * 增加数据爬取规则：更新 data/crawler/rules.py。

## 演示截图

主界面

未来规划

* 支持语音输入与结果语音输出。
* 增加多语言支持（如中文、英文）。
* 实现更高效的模型优化与自适应学习能力。

## 贡献指南

欢迎对本项目贡献代码、文档或提出改进建议：

1. Fork 本项目。
2. 创建特性分支 (git checkout -b feature/AmazingFeature)。
3. 提交修改 (git commit -m 'Add some AmazingFeature')。
4. 推送到分支 (git push origin feature/AmazingFeature)。
5. 创建 Pull Request。

## 开源协议

本项目遵循 MIT License 开源协议。
