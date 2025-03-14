#include "CrawlerManager.h"
#include "../../log/Logger.h"

namespace IntelliSearch
{

    CrawlerManager::CrawlerManager(QObject *parent)
        : QObject(parent), m_crawler(std::make_unique<Crawler>()), m_isCrawling(false), m_crawledCount(0), m_totalCount(0)
    {
        // 连接爬虫信号到管理器槽函数
        connect(m_crawler.get(), &Crawler::progressChanged,
                this, &CrawlerManager::handleProgressChanged);
        connect(m_crawler.get(), &Crawler::statusChanged,
                this, &CrawlerManager::handleStatusChanged);
        connect(m_crawler.get(), &Crawler::resultReady,
                this, &CrawlerManager::handleResultReady);
        connect(m_crawler.get(), &Crawler::crawlingCompleted,
                this, &CrawlerManager::handleCrawlingCompleted);
        connect(m_crawler.get(), &Crawler::errorOccurred,
                this, &CrawlerManager::handleErrorOccurred);

        INFOLOG("CrawlerManager initialized");
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
        m_totalCount = 0;

        // 开始爬取
        m_crawler->startCrawling(urls);

        INFOLOG("Started crawling with {} URLs", urls.size());
    }

    void CrawlerManager::pauseCrawling()
    {
        if (m_isCrawling)
        {
            m_crawler->pauseCrawling();
            INFOLOG("Paused crawling");
        }
    }

    void CrawlerManager::resumeCrawling()
    {
        if (!m_isCrawling)
        {
            m_crawler->resumeCrawling();
            INFOLOG("Resumed crawling");
        }
    }

    void CrawlerManager::stopCrawling()
    {
        m_crawler->stopCrawling();
        INFOLOG("Stopped crawling");
    }

    void CrawlerManager::setMaxDepth(int depth)
    {
        CrawlerConfig config = m_crawler->getConfig();
        config.maxDepth = depth;
        m_crawler->setConfig(config);
        INFOLOG("Set max depth to {}", depth);
    }

    void CrawlerManager::setMaxPages(int pages)
    {
        CrawlerConfig config = m_crawler->getConfig();
        config.maxPages = pages;
        m_crawler->setConfig(config);
        INFOLOG("Set max pages to {}", pages);
    }

    void CrawlerManager::setRequestDelay(int delay)
    {
        CrawlerConfig config = m_crawler->getConfig();
        config.requestDelay = delay;
        m_crawler->setConfig(config);
        INFOLOG("Set request delay to {} ms", delay);
    }

    void CrawlerManager::setFollowExternalLinks(bool follow)
    {
        CrawlerConfig config = m_crawler->getConfig();
        config.followExternalLinks = follow;
        m_crawler->setConfig(config);
        INFOLOG("Set follow external links to {}", follow);
    }

    void CrawlerManager::setAllowedDomains(const QStringList &domains)
    {
        CrawlerConfig config = m_crawler->getConfig();
        config.allowedDomains = domains;
        m_crawler->setConfig(config);
        INFOLOG("Set allowed domains: {}", domains.join(", ").toStdString());
    }

    void CrawlerManager::setUrlFilters(const QStringList &filters)
    {
        CrawlerConfig config = m_crawler->getConfig();
        config.urlFilters = filters;
        m_crawler->setConfig(config);
        INFOLOG("Set URL filters: {}", filters.join(", ").toStdString());
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

    void CrawlerManager::handleStatusChanged(CrawlerStatus status)
    {
        // 更新爬取状态
        m_isCrawling = (status == CrawlerStatus::Running);

        // 发送状态变化信号
        emit crawlingStatusChanged(m_isCrawling);
    }

    void CrawlerManager::handleResultReady(const CrawlResult &result)
    {
        // 添加到结果列表
        m_results.append(result);

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