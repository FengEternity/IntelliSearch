#include "CrawlerManager.h"
#include "PythonCrawlerBridge.h"
#include "../../log/Logger.h"

namespace IntelliSearch
{

    CrawlerManager::CrawlerManager(QObject *parent)
        : QObject(parent), m_pythonCrawler(std::make_unique<PythonCrawlerBridge>()), m_isCrawling(false),
          m_crawledCount(0), m_totalCount(0), m_maxThreads(4)
    {
        // 初始化线程池
        m_threadPool.setMaxThreadCount(m_maxThreads);
        // 启用动态爬取
        PythonCrawlerConfig config = m_pythonCrawler->getConfig();
        config.useDynamicCrawling = true;
        m_pythonCrawler->setConfig(config);

        // 连接爬虫信号到管理器槽函数
        connect(m_pythonCrawler.get(), &PythonCrawlerBridge::progressChanged,
                this, &CrawlerManager::handleProgressChanged);
        connect(m_pythonCrawler.get(), &PythonCrawlerBridge::statusChanged,
                this, &CrawlerManager::handleStatusChanged);
        connect(m_pythonCrawler.get(), &PythonCrawlerBridge::resultReady,
                this, &CrawlerManager::handleResultReady);
        connect(m_pythonCrawler.get(), &PythonCrawlerBridge::crawlingCompleted,
                this, &CrawlerManager::handleCrawlingCompleted);
        connect(m_pythonCrawler.get(), &PythonCrawlerBridge::errorOccurred,
                this, &CrawlerManager::handleErrorOccurred);

        INFOLOG("CrawlerManager initialized with Python crawler");
    }

    CrawlerManager::~CrawlerManager() = default;

    bool CrawlerManager::isCrawling() const
    {
        return m_isCrawling;
    }

    int CrawlerManager::getCrawledCount() const
    {
        return m_crawledCount;
    }

    int CrawlerManager::getTotalCount() const
    {
        return m_totalCount;
    }

    void CrawlerManager::startCrawling(const QString &url)
    {
        startCrawling(QStringList() << url);
    }

    void CrawlerManager::startCrawling(const QStringList &urls)
    {
        if (m_isCrawling)
        {
            WARNLOG("Crawler is already running");
            return;
        }

        // 清空之前的结果
        m_results.clear();
        m_crawledCount = 0;
        m_totalCount = urls.size();

        // 将URL添加到任务队列
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (const QString &url : urls)
            {
                m_taskQueue.push(url);
            }
        }

        // 启动爬虫任务
        m_isCrawling = true;
        emit crawlingStatusChanged(true);

        // 提交任务到线程池
        for (int i = 0; i < qMin(m_maxThreads, urls.size()); ++i)
        {
            if (!m_taskQueue.empty())
            {
                QString url;
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    url = m_taskQueue.front();
                    m_taskQueue.pop();
                }
                auto task = new CrawlerTask(url, m_pythonCrawler.get());
                m_threadPool.start(task);
            }
        }

        INFOLOG("Started crawling with {} URLs", urls.size());
    }

    void CrawlerManager::pauseCrawling()
    {
        if (m_isCrawling)
        {
            m_pythonCrawler->pauseCrawling();
            INFOLOG("Paused crawling");
        }
    }

    void CrawlerManager::resumeCrawling()
    {
        if (!m_isCrawling)
        {
            m_pythonCrawler->resumeCrawling();
            INFOLOG("Resumed crawling");
        }
    }

    void CrawlerManager::stopCrawling()
    {
        m_pythonCrawler->stopCrawling();
        m_threadPool.clear();
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::queue<QString> empty;
            std::swap(m_taskQueue, empty);
        }
        m_isCrawling = false;
        emit crawlingStatusChanged(false);
        INFOLOG("Stopped crawling");
    }

    void CrawlerManager::setMaxDepth(int depth)
    {
        PythonCrawlerConfig config = m_pythonCrawler->getConfig();
        config.maxDepth = depth;
        m_pythonCrawler->setConfig(config);
        INFOLOG("Set max depth to {}", depth);
    }

    void CrawlerManager::setMaxPages(int pages)
    {
        PythonCrawlerConfig config = m_pythonCrawler->getConfig();
        config.maxPages = pages;
        m_pythonCrawler->setConfig(config);
        INFOLOG("Set max pages to {}", pages);
    }

    void CrawlerManager::setRequestDelay(int delay)
    {
        PythonCrawlerConfig config = m_pythonCrawler->getConfig();
        config.requestDelay = delay;
        m_pythonCrawler->setConfig(config);
        INFOLOG("Set request delay to {} ms", delay);
    }

    void CrawlerManager::setFollowExternalLinks(bool follow)
    {
        PythonCrawlerConfig config = m_pythonCrawler->getConfig();
        config.followExternalLinks = follow;
        m_pythonCrawler->setConfig(config);
        INFOLOG("Set follow external links to {}", follow);
    }

    void CrawlerManager::setAllowedDomains(const QStringList &domains)
    {
        PythonCrawlerConfig config = m_pythonCrawler->getConfig();
        config.allowedDomains = domains;
        m_pythonCrawler->setConfig(config);
        INFOLOG("Set allowed domains: {}", domains.join(", ").toStdString());
    }

    void CrawlerManager::setUrlFilters(const QStringList &filters)
    {
        PythonCrawlerConfig config = m_pythonCrawler->getConfig();
        config.urlFilters = filters;
        m_pythonCrawler->setConfig(config);
        INFOLOG("Set URL filters: {}", filters.join(", ").toStdString());
    }

    void CrawlerManager::setUseDynamicCrawling(bool useDynamic)
    {
        PythonCrawlerConfig config = m_pythonCrawler->getConfig();
        config.useDynamicCrawling = useDynamic;
        m_pythonCrawler->setConfig(config);
        INFOLOG("Set use dynamic crawling to {}", useDynamic);
    }

    QVariantList CrawlerManager::getCrawlResults(int limit) const
    {
        QVariantList results;

        // 限制返回结果数量
        int count = qMin(limit, m_results.size());

        for (int i = 0; i < count; ++i)
        {
            const CrawlResult &result = m_results.at(i);

            QVariantMap resultMap;
            resultMap["url"] = result.url;
            resultMap["title"] = result.title;
            resultMap["content"] = result.content;
            resultMap["links"] = QVariant(result.links);
            resultMap["timestamp"] = result.timestamp;

            results.append(resultMap);
        }

        return results;
    }

    void CrawlerManager::handleProgressChanged(int crawled, int total)
    {
        m_crawledCount = crawled;
        m_totalCount = total;

        // 转发进度信号
        emit progressChanged(crawled, total);
    }

    void CrawlerManager::handleStatusChanged(PythonCrawlerStatus status)
    {
        // 更新爬取状态
        m_isCrawling = (status == PythonCrawlerStatus::Running);

        // 发送状态变化信号
        emit crawlingStatusChanged(m_isCrawling);
    }

    // 爬虫任务类实现
CrawlerManager::CrawlerTask::CrawlerTask(const QString& url, PythonCrawlerBridge* crawler)
    : m_url(url), m_crawler(crawler)
{
    setAutoDelete(true);
}

void CrawlerManager::CrawlerTask::run()
{
    m_crawler->startCrawling(m_url);
}

void CrawlerManager::handleResultReady(const CrawlResult &result)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        // 添加到结果列表
        m_results.append(result);
        m_crawledCount++;

        // 如果还有待处理的URL，继续提交任务
        if (!m_taskQueue.empty())
        {
            QString url = m_taskQueue.front();
            m_taskQueue.pop();
            auto task = new CrawlerTask(url, m_pythonCrawler.get());
            m_threadPool.start(task);
        }
        else if (m_crawledCount >= m_totalCount)
        {
            m_isCrawling = false;
            emit crawlingStatusChanged(false);
            emit crawlingCompleted();
        }
    }

    // 转换为QVariantMap并发送信号
    QVariantMap resultMap;
        resultMap["url"] = result.url;
        resultMap["title"] = result.title;
        resultMap["content"] = result.content;
        resultMap["links"] = QVariant(result.links);
        resultMap["timestamp"] = result.timestamp;

        emit resultReady(resultMap);
    }

    void CrawlerManager::handleCrawlingCompleted()
    {
        // 发送爬取完成信号
        emit crawlingCompleted();
        INFOLOG("Crawling completed, total results: {}", m_results.size());
    }

    void CrawlerManager::handleErrorOccurred(const QString &errorMessage)
    {
        // 转发错误信号
        emit errorOccurred(errorMessage);
        ERRORLOG("Crawler error: {}", errorMessage.toStdString());
    }

} // namespace IntelliSearch