#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
IntelliSearch 动态网页爬虫模块

该模块提供了一个用于爬取动态网页内容的爬虫实现，继承自基础Crawler类。
使用Selenium WebDriver来处理JavaScript渲染的内容，支持等待页面加载、执行脚本等功能。
"""

import time
import logging
from typing import List, Dict, Optional, Any
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.common.by import By
from selenium.common.exceptions import TimeoutException, WebDriverException
from webdriver_manager.chrome import ChromeDriverManager

# 导入基础爬虫模块
from crawler import Crawler, CrawlResult, CrawlerConfig, logger


class DynamicCrawlerConfig(CrawlerConfig):
    """动态爬虫配置类，扩展基础爬虫配置"""
    
    def __init__(self):
        super().__init__()
        # 默认启用动态爬取
        self.use_dynamic_crawling = True
        # WebDriver配置
        self.webdriver_path = ""  # WebDriver路径，如果为空则使用系统PATH中的WebDriver
        self.browser_type = "chrome"  # 浏览器类型：chrome, firefox, edge等
        self.headless = True  # 是否使用无头模式
        self.window_width = 1920  # 浏览器窗口宽度
        self.window_height = 1080  # 浏览器窗口高度
        self.load_images = False  # 是否加载图片
        self.wait_for_timeout = 10  # 等待元素出现的超时时间（秒）
        self.wait_for_selector = "body"  # 等待页面加载完成的选择器
        self.scroll_to_bottom = True  # 是否滚动到页面底部以加载懒加载内容
        self.scroll_pause_time = 1.0  # 滚动暂停时间（秒）
        self.execute_js = ""  # 在页面加载后执行的JavaScript代码
        self.browser_arguments = []  # 浏览器启动参数


class DynamicCrawler(Crawler):
    """动态爬虫类，使用Selenium WebDriver处理动态网页内容"""
    
    def __init__(self):
        super().__init__()
        self.config = DynamicCrawlerConfig()
        self.driver = None
    
    def _init_webdriver(self) -> None:
        """初始化WebDriver"""
        try:
            if self.config.browser_type.lower() == "chrome":
                options = Options()
                
                # 设置无头模式
                if self.config.headless:
                    options.add_argument("--headless")
                    options.add_argument("--no-sandbox")
                
                # 设置窗口大小
                options.add_argument(f"--window-size={self.config.window_width},{self.config.window_height}")
                
                # 不加载图片以提高性能
                if not self.config.load_images:
                    options.add_argument("--blink-settings=imagesEnabled=false")
                
                # 添加自定义浏览器参数
                for arg in self.config.browser_arguments:
                    options.add_argument(arg)
                
                # 设置用户代理
                options.add_argument(f"user-agent={self.config.user_agent}")
                
                # 创建WebDriver实例
                if self.config.webdriver_path:
                    service = Service(executable_path=self.config.webdriver_path)
                    self.driver = webdriver.Chrome(service=service, options=options)
                else:
                    service = Service(ChromeDriverManager().install())
                    self.driver = webdriver.Chrome(service=service, options=options)
                
                # 设置页面加载超时时间
                self.driver.set_page_load_timeout(self.config.page_load_timeout)
                
                logger.info("Chrome WebDriver初始化成功")
            
            elif self.config.browser_type.lower() == "firefox":
                # Firefox WebDriver配置
                from selenium.webdriver.firefox.options import Options as FirefoxOptions
                from selenium.webdriver.firefox.service import Service as FirefoxService
                
                options = FirefoxOptions()
                if self.config.headless:
                    options.add_argument("--headless")
                
                # 设置窗口大小
                options.add_argument(f"--width={self.config.window_width}")
                options.add_argument(f"--height={self.config.window_height}")
                
                # 添加自定义浏览器参数
                for arg in self.config.browser_arguments:
                    options.add_argument(arg)
                
                # 设置用户代理
                options.set_preference("general.useragent.override", self.config.user_agent)
                
                # 创建WebDriver实例
                if self.config.webdriver_path:
                    service = FirefoxService(executable_path=self.config.webdriver_path)
                    self.driver = webdriver.Firefox(service=service, options=options)
                else:
                    self.driver = webdriver.Firefox(options=options)
                
                # 设置页面加载超时时间
                self.driver.set_page_load_timeout(self.config.page_load_timeout)
                
                logger.info("Firefox WebDriver初始化成功")
            
            else:
                logger.error(f"不支持的浏览器类型: {self.config.browser_type}")
                raise ValueError(f"不支持的浏览器类型: {self.config.browser_type}")
            
        except Exception as e:
            logger.error(f"初始化WebDriver失败: {e}")
            raise
    
    def start_crawling(self, urls: List[str]) -> None:
        """开始爬取多个URL"""
        if self.is_running:
            logger.warning("爬虫已经在运行中")
            return
        
        try:
            # 初始化WebDriver
            self._init_webdriver()
            
            # 调用父类的start_crawling方法
            super().start_crawling(urls)
            
        except Exception as e:
            logger.error(f"动态爬取过程中发生错误: {e}")
        finally:
            # 关闭WebDriver
            self._close_webdriver()
    
    def _close_webdriver(self) -> None:
        """关闭WebDriver"""
        if self.driver:
            try:
                self.driver.quit()
                logger.info("WebDriver已关闭")
            except Exception as e:
                logger.error(f"关闭WebDriver失败: {e}")
            finally:
                self.driver = None
    
    def fetch_url(self, url: str) -> Optional[CrawlResult]:
        """使用WebDriver获取URL内容"""
        if not self.driver:
            logger.error("WebDriver未初始化")
            return None
        
        try:
            logger.info(f"使用WebDriver加载URL: {url}")
            
            # 加载页面
            self.driver.get(url)
            
            # 等待页面加载完成
            if self.config.wait_for_selector:
                try:
                    WebDriverWait(self.driver, self.config.wait_for_timeout).until(
                        EC.presence_of_element_located((By.CSS_SELECTOR, self.config.wait_for_selector))
                    )
                except TimeoutException:
                    logger.warning(f"等待选择器 '{self.config.wait_for_selector}' 超时")
            
            # 执行自定义JavaScript代码
            if self.config.execute_js:
                try:
                    self.driver.execute_script(self.config.execute_js)
                    logger.debug("执行自定义JavaScript代码成功")
                except Exception as e:
                    logger.error(f"执行JavaScript代码失败: {e}")
            
            # 滚动到页面底部以加载懒加载内容
            if self.config.scroll_to_bottom:
                self._scroll_to_bottom()
            
            # 获取页面内容
            html = self.driver.page_source
            
            # 解析HTML
            result = self.html_parser.parse_html(url, html)
            
            # 添加响应信息到元数据
            result.metadata['dynamic_crawled'] = True
            result.metadata['page_title'] = self.driver.title
            result.metadata['page_url'] = self.driver.current_url
            
            return result
            
        except WebDriverException as e:
            logger.error(f"WebDriver请求失败: {url}, 错误: {e}")
            return None
        except Exception as e:
            logger.error(f"动态爬取URL失败: {url}, 错误: {e}")
            return None
    
    def _scroll_to_bottom(self) -> None:
        """滚动到页面底部以加载懒加载内容"""
        try:
            # 获取初始页面高度
            last_height = self.driver.execute_script("return document.body.scrollHeight")
            
            while True:
                # 滚动到页面底部
                self.driver.execute_script("window.scrollTo(0, document.body.scrollHeight);")
                
                # 等待页面加载
                time.sleep(self.config.scroll_pause_time)
                
                # 计算新的页面高度
                new_height = self.driver.execute_script("return document.body.scrollHeight")
                
                # 如果页面高度没有变化，说明已经到达底部
                if new_height == last_height:
                    break
                
                last_height = new_height
                
            logger.debug("已滚动到页面底部")
            
        except Exception as e:
            logger.error(f"滚动页面失败: {e}")
    
    def take_screenshot(self, filepath: str) -> bool:
        """截取当前页面的屏幕截图"""
        if not self.driver:
            logger.error("WebDriver未初始化，无法截图")
            return False
        
        try:
            self.driver.save_screenshot(filepath)
            logger.info(f"截图已保存到: {filepath}")
            return True
        except Exception as e:
            logger.error(f"截图失败: {e}")
            return False
    
    def execute_script(self, script: str) -> Any:
        """在当前页面执行JavaScript代码"""
        if not self.driver:
            logger.error("WebDriver未初始化，无法执行脚本")
            return None
        
        try:
            result = self.driver.execute_script(script)
            return result
        except Exception as e:
            logger.error(f"执行脚本失败: {e}")
            return None


def main():
    """主函数"""
    import argparse
    
    # 解析命令行参数
    parser = argparse.ArgumentParser(description='IntelliSearch 动态网页爬虫')
    parser.add_argument('urls', nargs='+', help='要爬取的URL列表')
    parser.add_argument('--config', '-c', help='配置文件路径')
    parser.add_argument('--depth', '-d', type=int, help='最大爬取深度')
    parser.add_argument('--pages', '-p', type=int, help='最大爬取页面数')
    parser.add_argument('--delay', type=float, help='请求延迟(秒)')
    parser.add_argument('--headless', action='store_true', help='使用无头模式')
    parser.add_argument('--no-images', action='store_true', help='不加载图片')
    parser.add_argument('--scroll', action='store_true', help='滚动到页面底部')
    parser.add_argument('--output-dir', '-o', help='输出目录')
    
    args = parser.parse_args()
    
    # 创建动态爬虫实例
    crawler = DynamicCrawler()
    
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
    if args.headless is not None:
        crawler.config.headless = args.headless
    if args.no_images:
        crawler.config.load_images = False
    if args.scroll:
        crawler.config.scroll_to_bottom = True
    if args.output_dir:
        crawler.config.output_dir = args.output_dir
    
    # 开始爬取
    crawler.start_crawling(args.urls)


if __name__ == '__main__':
    main()