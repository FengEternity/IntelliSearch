#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
IntelliSearch Python爬虫模块

该模块提供了一个简单而强大的网页爬虫实现，可以抓取网页内容、解析HTML、提取链接和管理爬取队列。
支持配置爬取深度、延迟、URL过滤等参数。
"""

import requests
from bs4 import BeautifulSoup
import time
import json
import re
import os
import logging
import urllib.parse
from datetime import datetime
from collections import deque
from typing import List, Dict, Set, Optional, Any, Tuple

# 配置日志
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger('IntelliSearchCrawler')


class CrawlResult:
    """爬取结果类，存储单个页面的爬取结果"""
    
    def __init__(self, url: str):
        self.url = url
        self.title = ""
        self.content = ""
        self.links = []
        self.metadata = {}
        self.timestamp = datetime.now()
    
    def to_dict(self) -> Dict[str, Any]:
        """将爬取结果转换为字典格式"""
        return {
            'url': self.url,
            'title': self.title,
            'content': self.content,
            'links': self.links,
            'metadata': self.metadata,
            'timestamp': self.timestamp.isoformat()
        }
    
    def from_dict(self, data: Dict[str, Any]) -> None:
        """从字典加载爬取结果"""
        self.url = data.get('url', '')
        self.title = data.get('title', '')
        self.content = data.get('content', '')
        self.links = data.get('links', [])
        self.metadata = data.get('metadata', {})
        timestamp_str = data.get('timestamp')
        if timestamp_str:
            self.timestamp = datetime.fromisoformat(timestamp_str)
        else:
            self.timestamp = datetime.now()


class CrawlerConfig:
    """爬虫配置类，存储爬虫的配置参数"""
    
    def __init__(self):
        self.max_depth = 1                  # 最大爬取深度
        self.max_pages = 1                  # 最大爬取页面数
        self.request_delay = 1.0            # 请求延迟(秒)
        self.follow_external_links = False  # 是否跟随外部链接
        self.use_dynamic_crawling = False   # 是否使用动态爬取
        self.page_load_timeout = 30         # 页面加载超时时间（秒）
        self.allowed_domains = []           # 允许的域名列表
        self.url_filters = []               # URL过滤规则
        self.content_filters = []           # 内容过滤规则
        self.user_agent = 'IntelliSearch Python Crawler/1.0'  # 用户代理
        self.headers = {}                   # 请求头
        self.cookies = {}                   # Cookie
        self.proxies = {}                   # 代理设置
        self.output_dir = 'crawl_results'   # 输出目录
    
    def to_dict(self) -> Dict[str, Any]:
        """将配置转换为字典格式"""
        return self.__dict__.copy()
    
    def from_dict(self, data: Dict[str, Any]) -> None:
        """从字典加载配置"""
        for key, value in data.items():
            if hasattr(self, key):
                setattr(self, key, value)
    
    def load_from_file(self, filepath: str) -> None:
        """从文件加载配置"""
        try:
            with open(filepath, 'r', encoding='utf-8') as f:
                config_data = json.load(f)
                self.from_dict(config_data)
            logger.info(f"配置已从 {filepath} 加载")
        except Exception as e:
            logger.error(f"加载配置文件失败: {e}")
    
    def save_to_file(self, filepath: str) -> None:
        """保存配置到文件"""
        try:
            os.makedirs(os.path.dirname(filepath), exist_ok=True)
            with open(filepath, 'w', encoding='utf-8') as f:
                json.dump(self.to_dict(), f, indent=4, ensure_ascii=False)
            logger.info(f"配置已保存到 {filepath}")
        except Exception as e:
            logger.error(f"保存配置文件失败: {e}")


class HtmlParser:
    """HTML解析器类，用于解析HTML内容、提取链接和文本"""
    
    def __init__(self):
        pass
    
    def parse_html(self, url: str, html: str) -> CrawlResult:
        """解析HTML内容"""
        result = CrawlResult(url)
        
        try:
            soup = BeautifulSoup(html, 'html.parser')
            
            # 提取标题
            title_tag = soup.find('title')
            if title_tag:
                result.title = title_tag.text.strip()
            
            # 提取正文内容
            # 移除脚本和样式元素
            for script in soup(["script", "style", "nav", "footer", "header"]):
                script.extract()
            
            # 获取正文
            body = soup.find('body')
            if body:
                # 尝试找到主要内容区域
                main_content = body.find(['main', 'article', 'div', 'section'], 
                                        id=re.compile(r'content|main|article', re.I))
                if not main_content:
                    main_content = body.find(['main', 'article', 'div', 'section'], 
                                            class_=re.compile(r'content|main|article', re.I))
                
                if main_content:
                    result.content = main_content.get_text(separator='\n', strip=True)
                else:
                    # 如果找不到主要内容区域，使用整个body
                    result.content = body.get_text(separator='\n', strip=True)
            
            # 提取链接
            result.links = self.extract_links(url, html)
            
            # 提取元数据
            meta_tags = soup.find_all('meta')
            for tag in meta_tags:
                if tag.get('name') and tag.get('content'):
                    result.metadata[tag['name']] = tag['content']
            
            # 添加页面大小信息
            result.metadata['page_size_bytes'] = len(html)
            result.metadata['text_length'] = len(result.content)
            
        except Exception as e:
            logger.error(f"解析HTML失败: {e}")
        
        return result
    
    def extract_links(self, base_url: str, html: str) -> List[str]:
        """提取页面中的链接"""
        links = []
        try:
            soup = BeautifulSoup(html, 'html.parser')
            for a_tag in soup.find_all('a', href=True):
                href = a_tag['href']
                # 规范化URL
                normalized_url = self.normalize_url(base_url, href)
                if normalized_url:
                    links.append(normalized_url)
        except Exception as e:
            logger.error(f"提取链接失败: {e}")
        
        return list(set(links))  # 去重
    
    def normalize_url(self, base_url: str, url: str) -> Optional[str]:
        """规范化URL"""
        try:
            # 忽略空链接、锚点链接和JavaScript链接
            if not url or url.startswith('#') or url.startswith('javascript:'):
                return None
            
            # 处理相对URL
            full_url = urllib.parse.urljoin(base_url, url)
            
            # 移除URL中的锚点部分
            parsed_url = urllib.parse.urlparse(full_url)
            clean_url = urllib.parse.urlunparse((parsed_url.scheme, 
                                               parsed_url.netloc, 
                                               parsed_url.path,
                                               parsed_url.params,
                                               parsed_url.query,
                                               ''))
            
            return clean_url
        except Exception as e:
            logger.error(f"规范化URL失败: {e}")
            return None


class Crawler:
    """爬虫类，实现网页爬取功能"""
    
    def __init__(self):
        self.config = CrawlerConfig()
        self.html_parser = HtmlParser()
        self.url_queue = deque()  # 待爬取URL队列
        self.crawled_urls = set()  # 已爬取URL集合
        self.pending_urls = set()  # 正在爬取的URL集合
        self.url_depth_map = {}  # URL深度映射
        self.results = []  # 爬取结果列表
        self.is_running = False  # 爬虫运行状态
        self.session = requests.Session()  # 请求会话
    
    def start_crawling(self, urls: List[str]) -> None:
        """开始爬取多个URL"""
        if self.is_running:
            logger.warning("爬虫已经在运行中")
            return
        
        # 重置爬虫状态
        self.url_queue.clear()
        self.crawled_urls.clear()
        self.pending_urls.clear()
        self.url_depth_map.clear()
        self.results = []
        
        # 设置请求头
        self.session.headers.update({
            'User-Agent': self.config.user_agent,
            **self.config.headers
        })
        
        # 设置Cookie
        if self.config.cookies:
            self.session.cookies.update(self.config.cookies)
        
        # 添加初始URL到队列
        for url in urls:
            normalized_url = self.html_parser.normalize_url("", url)
            if normalized_url:
                self.url_queue.append(normalized_url)
                self.url_depth_map[normalized_url] = 0  # 初始深度为0
        
        if not self.url_queue:
            logger.error("没有有效的URL可爬取")
            return
        
        # 更新状态并开始爬取
        self.is_running = True
        logger.info(f"开始爬取 {len(urls)} 个初始URL")
        
        try:
            # 开始爬取循环
            while self.url_queue and self.is_running:
                # 检查是否达到最大页面数限制
                if self.config.max_pages > 0 and len(self.crawled_urls) >= self.config.max_pages:
                    logger.info(f"已达到最大页面数限制 ({self.config.max_pages})")
                    break
                
                # 获取下一个URL
                url = self.url_queue.popleft()
                
                # 检查URL是否已爬取或正在爬取
                if url in self.crawled_urls or url in self.pending_urls:
                    continue
                
                # 检查URL是否应该被爬取
                if not self.should_crawl(url):
                    continue
                
                # 添加到正在爬取集合
                self.pending_urls.add(url)
                
                # 获取当前URL的深度
                current_depth = self.url_depth_map.get(url, 0)
                
                # 爬取URL
                try:
                    logger.info(f"爬取URL: {url} (深度: {current_depth})")
                    result = self.fetch_url(url)
                    
                    if result:
                        # 处理爬取结果
                        self.process_result(result, current_depth)
                        
                        # 添加到已爬取集合
                        self.crawled_urls.add(url)
                        
                        # 输出进度
                        progress = f"进度: {len(self.crawled_urls)}/{len(self.crawled_urls) + len(self.url_queue)}"
                        logger.info(progress)
                    
                except Exception as e:
                    logger.error(f"爬取URL失败: {url}, 错误: {e}")
                
                # 从正在爬取集合中移除
                self.pending_urls.remove(url)
                
                # 延迟请求
                if self.config.request_delay > 0:
                    time.sleep(self.config.request_delay)
            
            # 爬取完成
            if self.is_running:
                logger.info(f"爬取完成，共爬取 {len(self.crawled_urls)} 个页面")
                self.save_results()
        
        except KeyboardInterrupt:
            logger.info("爬取被用户中断")
        except Exception as e:
            logger.error(f"爬取过程中发生错误: {e}")
        finally:
            self.is_running = False
    
    def fetch_url(self, url: str) -> Optional[CrawlResult]:
        """获取URL内容"""
        try:
            # 发送HTTP请求
            response = self.session.get(url, timeout=self.config.page_load_timeout, proxies=self.config.proxies)
            response.raise_for_status()  # 检查响应状态
            
            # 获取响应内容
            html = response.text
            
            # 解析HTML
            result = self.html_parser.parse_html(url, html)
            
            # 添加响应信息到元数据
            result.metadata['status_code'] = response.status_code
            result.metadata['content_type'] = response.headers.get('Content-Type', '')
            result.metadata['encoding'] = response.encoding
            
            return result
            
        except requests.RequestException as e:
            logger.error(f"请求失败: {url}, 错误: {e}")
            return None
    
    def should_crawl(self, url: str) -> bool:
        """检查URL是否应该被爬取"""
        try:
            # 解析URL
            parsed_url = urllib.parse.urlparse(url)
            domain = parsed_url.netloc
            
            # 检查域名是否在允许列表中
            if self.config.allowed_domains and domain not in self.config.allowed_domains:
                logger.debug(f"跳过URL: {url}, 域名不在允许列表中")
                return False
            
            # 检查URL是否匹配过滤规则
            for pattern in self.config.url_filters:
                if re.search(pattern, url):
                    logger.debug(f"跳过URL: {url}, 匹配过滤规则: {pattern}")
                    return False
            
            return True
            
        except Exception as e:
            logger.error(f"检查URL失败: {url}, 错误: {e}")
            return False
    
    def process_result(self, result: CrawlResult, current_depth: int) -> None:
        """处理爬取结果"""
        # 添加结果到列表
        self.results.append(result)
        
        # 检查是否达到最大深度
        if self.config.max_depth >= 0 and current_depth >= self.config.max_depth:
            logger.debug(f"达到最大深度 ({self.config.max_depth})，不再继续爬取链接")
            return
        
        # 处理链接
        for link in result.links:
            # 检查链接是否已在队列中或已爬取
            if link in self.crawled_urls or link in self.pending_urls or link in self.url_queue:
                continue
            
            # 检查是否跟随外部链接
            if not self.config.follow_external_links:
                # 检查链接是否为外部链接
                parsed_result_url = urllib.parse.urlparse(result.url)
                parsed_link_url = urllib.parse.urlparse(link)
                
                if parsed_result_url.netloc != parsed_link_url.netloc:
                    logger.debug(f"跳过外部链接: {link}")
                    continue
            
            # 添加链接到队列
            self.url_queue.append(link)
            
            # 设置链接深度
            self.url_depth_map[link] = current_depth + 1
    
    def save_results(self) -> None:
        """保存爬取结果"""
        try:
            # 创建输出目录
            os.makedirs(self.config.output_dir, exist_ok=True)
            
            # 保存结果到JSON文件
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"crawl_results_{timestamp}.json"
            filepath = os.path.join(self.config.output_dir, filename)
            
            # 转换结果为字典列表
            results_dict = [result.to_dict() for result in self.results]
            
            # 写入文件
            with open(filepath, 'w', encoding='utf-8') as f:
                json.dump(results_dict, f, indent=4, ensure_ascii=False)
            
            logger.info(f"结果已保存到 {filepath}")
            
        except Exception as e:
            logger.error(f"保存结果失败: {e}")
    
    def stop_crawling(self) -> None:
        """停止爬取"""
        if self.is_running:
            logger.info("停止爬取")
            self.is_running = False
    
    def get_stats(self) -> Dict[str, Any]:
        """获取爬虫统计信息"""
        return {
            'crawled_count': len(self.crawled_urls),
            'queue_count': len(self.url_queue),
            'pending_count': len(self.pending_urls),
            'total_count': len(self.crawled_urls) + len(self.url_queue) + len(self.pending_urls),
            'is_running': self.is_running
        }


def main():
    """主函数"""
    import argparse
    
    # 解析命令行参数
    parser = argparse.ArgumentParser(description='IntelliSearch Python爬虫')
    parser.add_argument('urls', nargs='+', help='要爬取的URL列表')
    parser.add_argument('--config', '-c', help='配置文件路径')
    parser.add_argument('--depth', '-d', type=int, help='最大爬取深度')
    parser.add_argument('--pages', '-p', type=int, help='最大爬取页面数')
    parser.add_argument('--delay', type=float, help='请求延迟(秒)')
    parser.add_argument('--follow-external', action='store_true', help='是否跟随外部链接')
    parser.add_argument('--output-dir', '-o', help='输出目录')
    
    args = parser.parse_args()
    
    # 创建爬虫实例
    crawler = Crawler()
    
    # 加载配置文件
    if args.config:
        crawler.config.load_from_file(args.config)
    
    # 应用命令行参数
    if args.depth is not None:
        crawler.config.max_depth = args.depth
    if args.pages is not None:
        crawler.config.max_pages = args.pages
    if args.delay is not None:
        crawler.config.request_delay = args.delay
    if args.follow_external:
        crawler.config.follow_external_links = True
    if args.output_dir:
        crawler.config.output_dir = args.output_dir
    
    # 开始爬取
    crawler.start_crawling(args.urls)


if __name__ == '__main__':
    main()