#include "JSExecutionAnalyzer.h"
#include "WebEnginePage.h"
#include "../../log/Logger.h"

namespace IntelliSearch
{

    JSExecutionAnalyzer::JSExecutionAnalyzer(QObject *parent) : QObject(parent)
    {
        // 初始化监控脚本
        m_monitoringScript = R"(
            // 创建监控对象
            window.__intelliSearchMonitor = {
                domMutations: [],
                ajaxRequests: [],
                eventListeners: 0,
                timers: 0,
                storageAccess: 0,
                networkRequests: []
            };
            
            // 监控DOM变化
            (function() {
                const observer = new MutationObserver(mutations => {
                    mutations.forEach(mutation => {
                        window.__intelliSearchMonitor.domMutations.push({
                            type: mutation.type,
                            target: mutation.target.nodeName,
                            addedNodes: mutation.addedNodes.length,
                            removedNodes: mutation.removedNodes.length,
                            timestamp: Date.now()
                        });
                    });
                });
                
                // 开始观察文档
                observer.observe(document, {
                    childList: true,
                    subtree: true,
                    attributes: true,
                    characterData: true
                });
            })();
            
            // 监控AJAX请求
            (function() {
                const originalXHR = window.XMLHttpRequest;
                function newXHR() {
                    const xhr = new originalXHR();
                    const originalOpen = xhr.open;
                    const originalSend = xhr.send;
                    
                    xhr.open = function() {
                        const method = arguments[0];
                        const url = arguments[1];
                        window.__intelliSearchMonitor.ajaxRequests.push({
                            method: method,
                            url: url,
                            timestamp: Date.now()
                        });
                        return originalOpen.apply(this, arguments);
                    };
                    
                    return xhr;
                }
                window.XMLHttpRequest = newXHR;
                
                // 监控Fetch API
                const originalFetch = window.fetch;
                window.fetch = function() {
                    const url = arguments[0];
                    const options = arguments[1] || {};
                    window.__intelliSearchMonitor.ajaxRequests.push({
                        method: options.method || 'GET',
                        url: typeof url === 'string' ? url : url.url,
                        timestamp: Date.now()
                    });
                    return originalFetch.apply(this, arguments);
                };
            })();
            
            // 监控事件监听器
            (function() {
                const originalAddEventListener = EventTarget.prototype.addEventListener;
                EventTarget.prototype.addEventListener = function() {
                    window.__intelliSearchMonitor.eventListeners++;
                    return originalAddEventListener.apply(this, arguments);
                };
            })();
            
            // 监控定时器
            (function() {
                const originalSetTimeout = window.setTimeout;
                window.setTimeout = function() {
                    window.__intelliSearchMonitor.timers++;
                    return originalSetTimeout.apply(this, arguments);
                };
                
                const originalSetInterval = window.setInterval;
                window.setInterval = function() {
                    window.__intelliSearchMonitor.timers++;
                    return originalSetInterval.apply(this, arguments);
                };
            })();
            
            // 监控存储访问
            (function() {
                const originalLocalStorageGetItem = Storage.prototype.getItem;
                const originalLocalStorageSetItem = Storage.prototype.setItem;
                
                Storage.prototype.getItem = function() {
                    window.__intelliSearchMonitor.storageAccess++;
                    return originalLocalStorageGetItem.apply(this, arguments);
                };
                
                Storage.prototype.setItem = function() {
                    window.__intelliSearchMonitor.storageAccess++;
                    return originalLocalStorageSetItem.apply(this, arguments);
                };
            })();
            
            // 监控网络请求
            (function() {
                if (window.PerformanceObserver) {
                    try {
                        const observer = new PerformanceObserver((list) => {
                            for (const entry of list.getEntries()) {
                                if (entry.initiatorType && ['fetch', 'xmlhttprequest', 'beacon', 'script'].includes(entry.initiatorType)) {
                                    window.__intelliSearchMonitor.networkRequests.push({
                                        url: entry.name,
                                        type: entry.initiatorType,
                                        duration: entry.duration,
                                        timestamp: Date.now()
                                    });
                                }
                            }
                        });
                        observer.observe({entryTypes: ['resource']});
                    } catch (e) {
                        console.error('Performance observer error:', e);
                    }
                }
            })();
        )";
        
        INFOLOG("JSExecutionAnalyzer initialized");
    }

    JSExecutionAnalyzer::~JSExecutionAnalyzer()
    {
        // 析构函数实现
    }

    JSExecutionResult JSExecutionAnalyzer::analyzeExecution(const QString &url, int timeout)
    {
        INFOLOG("Analyzing JavaScript execution for URL: {}", url.toStdString());
        
        // 创建WebEnginePage实例
        WebEnginePage *page = new WebEnginePage();
        
        // 加载URL
        page->load(QUrl(url));
        
        // 等待页面加载完成
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        
        bool loadFinished = false;
        
        // 连接页面加载完成信号
        QObject::connect(page, &QWebEnginePage::loadFinished, [&](bool success) {
            loadFinished = success;
            if (success) {
                // 注入监控脚本
                injectMonitoringScripts(page);
                
                // 等待JavaScript执行
                waitForExecution(page, timeout / 2);
            }
            loop.quit();
        });
        
        // 连接超时信号
        QObject::connect(&timer, &QTimer::timeout, [&]() {
            WARNLOG("Page load timeout");
            loop.quit();
        });
        
        // 启动超时计时器
        timer.start(timeout);
        
        // 等待页面加载完成或超时
        loop.exec();
        
        JSExecutionResult result;
        
        if (loadFinished) {
            // 获取初始DOM快照
            QString beforeSnapshot = captureSnapshot(page);
            
            // 等待一段时间，让JavaScript执行
            QTimer::singleShot(timeout / 2, [&]() {
                // 获取执行后的DOM快照
                QString afterSnapshot = captureSnapshot(page);
                
                // 比较快照差异
                double changeDegree = compareSnapshots(beforeSnapshot, afterSnapshot);
                
                // 收集执行结果
                result = collectResults(page);
                
                // 根据DOM变化程度判断是否有动态内容
                result.hasDynamicContent = (changeDegree > 0.1) || 
                                          (result.domMutationCount > 5) || 
                                          (result.ajaxRequestCount > 0) || 
                                          (result.timerCount > 3);
                
                INFOLOG("JavaScript execution analysis completed. Dynamic content: {}", 
                       result.hasDynamicContent ? "true" : "false");
                
                // 清理资源
                page->deleteLater();
                
                loop.quit();
            });
            
            // 等待分析完成
            loop.exec();
        } else {
            ERRORLOG("Failed to load page for JavaScript analysis: {}", url.toStdString());
            page->deleteLater();
            
            // 设置默认结果
            result.hasDynamicContent = false;
        }
        
        return result;
    }

    void JSExecutionAnalyzer::injectMonitoringScripts(QWebEnginePage *page)
    {
        // 注入监控脚本
        page->runJavaScript(m_monitoringScript, QWebEngineScript::ApplicationWorld);
        DEBUGLOG("Monitoring scripts injected");
    }

    JSExecutionResult JSExecutionAnalyzer::collectResults(QWebEnginePage *page)
    {
        JSExecutionResult result;
        QEventLoop loop;
        
        // 收集DOM变化次数
        page->runJavaScript("window.__intelliSearchMonitor.domMutations.length", 
                          QWebEngineScript::ApplicationWorld, 
                          [&](const QVariant &value) {
            result.domMutationCount = value.toInt();
            DEBUGLOG("DOM mutation count: {}", result.domMutationCount);
            loop.quit();
        });
        loop.exec();
        
        // 收集AJAX请求次数
        page->runJavaScript("window.__intelliSearchMonitor.ajaxRequests.length", 
                          QWebEngineScript::ApplicationWorld, 
                          [&](const QVariant &value) {
            result.ajaxRequestCount = value.toInt();
            DEBUGLOG("AJAX request count: {}", result.ajaxRequestCount);
            loop.quit();
        });
        loop.exec();
        
        // 收集事件监听器数量
        page->runJavaScript("window.__intelliSearchMonitor.eventListeners", 
                          QWebEngineScript::ApplicationWorld, 
                          [&](const QVariant &value) {
            result.eventListenerCount = value.toInt();
            DEBUGLOG("Event listener count: {}", result.eventListenerCount);
            loop.quit();
        });
        loop.exec();
        
        // 收集定时器数量
        page->runJavaScript("window.__intelliSearchMonitor.timers", 
                          QWebEngineScript::ApplicationWorld, 
                          [&](const QVariant &value) {
            result.timerCount = value.toInt();
            DEBUGLOG("Timer count: {}", result.timerCount);
            loop.quit();
        });
        loop.exec();
        
        // 收集存储访问次数
        page->runJavaScript("window.__intelliSearchMonitor.storageAccess", 
                          QWebEngineScript::ApplicationWorld, 
                          [&](const QVariant &value) {
            result.storageAccessCount = value.toInt();
            DEBUGLOG("Storage access count: {}", result.storageAccessCount);
            loop.quit();
        });
        loop.exec();
        
        // 收集网络请求详情
        page->runJavaScript("JSON.stringify(window.__intelliSearchMonitor.networkRequests)", 
                          QWebEngineScript::ApplicationWorld, 
                          [&](const QVariant &value) {
            QJsonDocument doc = QJsonDocument::fromJson(value.toString().toUtf8());
            result.networkRequests = doc.array();
            DEBUGLOG("Network requests collected: {}", result.networkRequests.size());
            loop.quit();
        });
        loop.exec();
        
        // 收集DOM变化详情
        page->runJavaScript("JSON.stringify(window.__intelliSearchMonitor.domMutations)", 
                          QWebEngineScript::ApplicationWorld, 
                          [&](const QVariant &value) {
            QJsonDocument doc = QJsonDocument::fromJson(value.toString().toUtf8());
            result.domChanges = doc.array();
            DEBUGLOG("DOM changes collected: {}", result.domChanges.size());
            loop.quit();
        });
        loop.exec();
        
        return result;
    }

    void JSExecutionAnalyzer::waitForExecution(QWebEnginePage *page, int timeout)
    {
        // 等待JavaScript执行
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        
        // 连接超时信号
        QObject::connect(&timer, &QTimer::timeout, [&]() {
            loop.quit();
        });
        
        // 启动超时计时器
        timer.start(timeout);
        
        // 等待超时
        loop.exec();
    }

    QString JSExecutionAnalyzer::captureSnapshot(QWebEnginePage *page)
    {
        QString snapshot;
        QEventLoop loop;
        
        // 获取页面HTML
        page->toHtml([&](const QString &html) {
            snapshot = html;
            loop.quit();
        });
        
        // 等待HTML获取完成
        loop.exec();
        
        return snapshot;
    }

    double JSExecutionAnalyzer::compareSnapshots(const QString &before, const QString &after)
    {
        // 简单比较：计算字符差异比例
        int beforeLength = before.length();
        int afterLength = after.length();
        
        // 如果长度相同，进一步比较内容
        if (beforeLength == afterLength && before == after) {
            return 0.0; // 完全相同
        }
        
        // 计算长度变化比例
        double lengthDiff = qAbs(afterLength - beforeLength) / static_cast<double>(beforeLength);
        
        // 计算内容差异（简化版，实际应用中可以使用更复杂的差异算法）
        int diffCount = 0;
        int minLength = qMin(beforeLength, afterLength);
        
        for (int i = 0; i < minLength; i++) {
            if (before[i] != after[i]) {
                diffCount++;
            }
        }
        
        double contentDiff = diffCount / static_cast<double>(minLength);
        
        // 综合长度差异和内容差异
        double changeDegree = (lengthDiff + contentDiff) / 2.0;
        
        DEBUGLOG("DOM snapshot comparison - Length diff: {}, Content diff: {}, Change degree: {}", 
               lengthDiff, contentDiff, changeDegree);
        
        return changeDegree;
    }

} // namespace IntelliSearch