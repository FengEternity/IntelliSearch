# IntelliSearch Python爬虫

这是一个简单而强大的网页爬虫实现，可以抓取网页内容、解析HTML、提取链接和管理爬取队列。支持配置爬取深度、延迟、URL过滤等参数。

## 功能特点

- 支持配置最大爬取深度和页面数量
- 支持设置请求延迟，避免对目标服务器造成压力
- 支持URL过滤和域名限制
- 智能提取网页主要内容
- 自动提取页面链接和元数据
- 结果保存为JSON格式，方便后续处理
- 完善的日志记录

## 安装依赖

```bash
pip install requests beautifulsoup4
```

## 使用方法

### 命令行使用

```bash
python crawler.py https://example.com --depth 2 --pages 100 --delay 1.0 --output-dir results
```

参数说明：
- `urls`：要爬取的URL列表（必需）
- `--config, -c`：配置文件路径
- `--depth, -d`：最大爬取深度
- `--pages, -p`：最大爬取页面数
- `--delay`：请求延迟(秒)
- `--follow-external`：是否跟随外部链接
- `--output-dir, -o`：输出目录

### 作为模块导入

```python
from crawler import Crawler, CrawlerConfig

# 创建爬虫实例
crawler = Crawler()

# 配置爬虫参数
crawler.config.max_depth = 2
crawler.config.max_pages = 100
crawler.config.request_delay = 1.0
crawler.config.follow_external_links = False
crawler.config.allowed_domains = ['example.com']
crawler.config.output_dir = 'results'

# 开始爬取
crawler.start_crawling(['https://example.com'])

# 获取爬虫统计信息
stats = crawler.get_stats()
print(stats)
```

## 配置文件示例

可以通过JSON文件配置爬虫参数：

```json
{
    "max_depth": 2,
    "max_pages": 100,
    "request_delay": 1.0,
    "follow_external_links": false,
    "use_dynamic_crawling": false,
    "page_load_timeout": 30,
    "allowed_domains": ["example.com"],
    "url_filters": [".*\\.pdf$", ".*\\.zip$"],
    "user_agent": "IntelliSearch Python Crawler/1.0",
    "output_dir": "crawl_results"
}
```

## 爬取结果格式

爬取结果保存为JSON文件，格式如下：

```json
[
    {
        "url": "https://example.com",
        "title": "Example Domain",
        "content": "This domain is for use in illustrative examples in documents...",
        "links": [
            "https://www.iana.org/domains/example"
        ],
        "metadata": {
            "description": "Example Domain",
            "page_size_bytes": 1256,
            "text_length": 165,
            "status_code": 200,
            "content_type": "text/html; charset=UTF-8",
            "encoding": "utf-8"
        },
        "timestamp": "2023-06-01T12:34:56.789012"
    }
]
```

## 与C++爬虫系统集成

本Python爬虫可以作为IntelliSearch C++爬虫系统的补充或替代方案。可以通过以下方式集成：

1. 作为独立工具运行，将结果保存到指定目录
2. 通过JSON文件交换配置和结果数据
3. 使用Python的C/C++扩展接口直接集成到C++系统中

## 注意事项

- 请遵守网站的robots.txt规则
- 设置合理的请求延迟，避免对目标服务器造成压力
- 对于需要JavaScript渲染的网页，本爬虫可能无法获取完整内容