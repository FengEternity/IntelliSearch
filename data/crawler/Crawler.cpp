#include "Crawler.h"
#include "../../log/Logger.h"
#include <QTextDocument>

namespace IntelliSearch
{

    Crawler::Crawler(QObject *parent)
        : QObject(parent), m_networkManager(new QNetworkAccessManager(this)), m_status(CrawlerStatus::Idle), m_requestTimer(new QTimer(this)), m_currentDepth(0)
    {
        // 连接网络请求完成信号
        connect(m_networkManager, &QNetworkAccessManager::finished,
                this, &Crawler::handleNetworkReply);

        // 连接请求定时器
        connect(m_requestTimer, &QTimer::timeout,
                this, &Crawler::requestNextUrl);

        // 设置默认配置
        m_config.maxDepth = 0;
        m_config.maxPages = 1;
        m_config.requestDelay = 1000;
        m_config.followExternalLinks = false;

        INFOLOG("Crawler initialized");
    }

    Crawler::~Crawler()
    {
        stopCrawling();
    }

    void Crawler::startCrawling(const QString &url)
    {
        startCrawling(QStringList() << url);
    }

    void Crawler::startCrawling(const QStringList &urls)
    {
        if (m_status == CrawlerStatus::Running)
        {
            WARNLOG("Crawler is already running");
            return;
        }

        // 重置爬虫状态
        m_urlQueue.clear();
        m_crawledUrls.clear();
        m_pendingUrls.clear();
        m_urlDepthMap.clear();
        m_currentDepth = 0;

        // 添加初始URL到队列
        for (const QString &url : urls)
        {
            QString normalizedUrl = normalizeUrl("", url);
            if (!normalizedUrl.isEmpty())
            {
                m_urlQueue.enqueue(normalizedUrl);
                m_urlDepthMap[normalizedUrl] = 0; // 初始深度为0
            }
        }

        if (m_urlQueue.isEmpty())
        {
            ERRORLOG("No valid URLs to crawl");
            emit errorOccurred("No valid URLs to crawl");
            return;
        }

        // 更新状态并开始爬取
        m_status = CrawlerStatus::Running;
        emit statusChanged(m_status);

        INFOLOG("Starting crawler with {} initial URLs", urls.size());

        // 开始请求
        requestNextUrl();
    }

    void Crawler::pauseCrawling()
    {
        if (m_status == CrawlerStatus::Running)
        {
            m_status = CrawlerStatus::Paused;
            m_requestTimer->stop();
            emit statusChanged(m_status);
            INFOLOG("Crawler paused");
        }
    }

    void Crawler::resumeCrawling()
    {
        if (m_status == CrawlerStatus::Paused)
        {
            m_status = CrawlerStatus::Running;
            requestNextUrl();
            emit statusChanged(m_status);
            INFOLOG("Crawler resumed");
        }
    }

    void Crawler::stopCrawling()
    {
        m_requestTimer->stop();

        // 取消所有待处理的请求
        QList<QNetworkReply *> replies = m_networkManager->findChildren<QNetworkReply *>();
        for (QNetworkReply *reply : replies)
        {
            reply->abort();
            reply->deleteLater();
        }

        m_urlQueue.clear();
        m_pendingUrls.clear();

        if (m_status != CrawlerStatus::Idle && m_status != CrawlerStatus::Completed)
        {
            m_status = CrawlerStatus::Idle;
            emit statusChanged(m_status);
            INFOLOG("Crawler stopped");
        }
    }

    void Crawler::setConfig(const CrawlerConfig &config)
    {
        m_config = config;
    }

    CrawlerConfig Crawler::getConfig() const
    {
        return m_config;
    }

    CrawlerStatus Crawler::getStatus() const
    {
        return m_status;
    }

    int Crawler::getCrawledCount() const
    {
        return m_crawledUrls.size();
    }

    void Crawler::handleNetworkReply(QNetworkReply *reply)
    {
        QString url = reply->url().toString();
        m_pendingUrls.remove(url);

        if (reply->error() == QNetworkReply::NoError)
        {
            // 读取响应内容
            QByteArray data = reply->readAll();
            QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();

            // 只处理HTML内容
            if (contentType.contains("text/html", Qt::CaseInsensitive))
            {
                QString html = QString::fromUtf8(data);

                // 解析HTML
                CrawlResult result = parseHtml(url, html);

                // 处理结果
                processResult(result);

                // 添加到已爬取集合
                m_crawledUrls.insert(url);

                // 发送进度信号
                emit progressChanged(m_crawledUrls.size(), m_crawledUrls.size() + m_urlQueue.size());

                DEBUGLOG("Crawled URL: {}", url.toStdString());
            }
            else
            {
                DEBUGLOG("Skipped non-HTML content: {} ({})", url.toStdString(), contentType.toStdString());
            }
        }
        else
        {
            QString errorMsg = reply->errorString();
            ERRORLOG("Network error for URL {}: {}", url.toStdString(), errorMsg.toStdString());
            emit errorOccurred(QString("Error crawling %1: %2").arg(url, errorMsg));
        }

        reply->deleteLater();

        // 检查是否完成
        if (m_pendingUrls.isEmpty() && m_urlQueue.isEmpty())
        {
            m_status = CrawlerStatus::Completed;
            emit statusChanged(m_status);
            emit crawlingCompleted();
            INFOLOG("Crawler completed, crawled {} URLs", m_crawledUrls.size());
        }
        else
        {
            // 请求下一个URL
            if (!m_requestTimer->isActive() && m_status == CrawlerStatus::Running)
            {
                m_requestTimer->start(m_config.requestDelay);
            }
        }
    }

    CrawlResult Crawler::parseHtml(const QString &url, const QString &html)
    {
        CrawlResult result;
        result.url = url;
        result.timestamp = QDateTime::currentDateTime();

        // 提取标题
        QRegularExpression titleRegex("<title>([^<]*)</title>", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch titleMatch = titleRegex.match(html);
        if (titleMatch.hasMatch())
        {
            result.title = titleMatch.captured(1).trimmed();
        }

        // 提取内容 (简单实现，移除HTML标签)
        QTextDocument doc;
        doc.setHtml(html);
        result.content = doc.toPlainText();

        // 提取链接
        result.links = extractLinks(url, html);

        INFOLOG("Parsed HTML for URL: {}", url.toStdString());
        INFOLOG("Parser result - Title: {}, Content length: {}, Links count: {}, content: {}", 
            result.title.toStdString(), 
            result.content.length(), 
            result.links.size(),
            result.content.toStdString()
        );

        return result;
    }

    QStringList Crawler::extractLinks(const QString &baseUrl, const QString &html)
    {
        QStringList links;
        QSet<QString> uniqueLinks;

        // 提取href属性
        QRegularExpression hrefRegex("href=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator i = hrefRegex.globalMatch(html);

        while (i.hasNext())
        {
            QRegularExpressionMatch match = i.next();
            QString href = match.captured(1).trimmed();

            // 规范化URL
            QString normalizedUrl = normalizeUrl(baseUrl, href);

            // 检查URL是否有效且未被处理过
            if (!normalizedUrl.isEmpty() && !uniqueLinks.contains(normalizedUrl))
            {
                uniqueLinks.insert(normalizedUrl);
                links.append(normalizedUrl);
            }
        }

        return links;
    }

    QString Crawler::normalizeUrl(const QString &baseUrl, const QString &url)
    {
        // 创建QUrl对象
        QUrl qurl;
        if (url.startsWith("http://") || url.startsWith("https://"))
        {
            qurl = QUrl(url);
        }
        else
        {
            QUrl base(baseUrl);
            qurl = base.resolved(QUrl(url));
        }

        // 检查URL是否有效
        if (!qurl.isValid())
        {
            return QString();
        }

        // 移除片段(#后面的内容)
        qurl.setFragment(QString());

        // 获取规范化的URL字符串
        QString normalizedUrl = qurl.toString();

        // 检查是否应该跟随外部链接
        if (!m_config.followExternalLinks && !baseUrl.isEmpty())
        {
            QUrl baseQUrl(baseUrl);
            if (baseQUrl.host() != qurl.host())
            {
                return QString();
            }
        }

        // 检查是否在允许的域名列表中
        if (!m_config.allowedDomains.isEmpty())
        {
            bool allowed = false;
            for (const QString &domain : m_config.allowedDomains)
            {
                if (qurl.host().endsWith(domain, Qt::CaseInsensitive))
                {
                    allowed = true;
                    break;
                }
            }
            if (!allowed)
            {
                return QString();
            }
        }

        // 应用URL过滤规则
        if (!m_config.urlFilters.isEmpty())
        {
            for (const QString &filter : m_config.urlFilters)
            {
                QRegularExpression regex(filter);
                if (regex.match(normalizedUrl).hasMatch())
                {
                    return QString();
                }
            }
        }

        return normalizedUrl;
    }

    bool Crawler::shouldCrawl(const QString &url)
    {
        // 检查URL是否已经爬取过或正在爬取
        if (m_crawledUrls.contains(url) || m_pendingUrls.contains(url))
        {
            return false;
        }

        // 检查是否达到最大页面数限制
        if (m_config.maxPages > 0 && m_crawledUrls.size() >= m_config.maxPages)
        {
            return false;
        }

        // 检查URL深度
        int depth = m_urlDepthMap.value(url, 0);
        if (m_config.maxDepth > 0 && depth > m_config.maxDepth)
        {
            return false;
        }

        return true;
    }

    void Crawler::processResult(const CrawlResult &result)
    {
        // 发送结果信号
        emit resultReady(result);

        // 获取当前URL的深度
        int currentDepth = m_urlDepthMap.value(result.url, 0);

        // 处理提取的链接
        for (const QString &link : result.links)
        {
            // 设置链接深度
            if (!m_urlDepthMap.contains(link))
            {
                m_urlDepthMap[link] = currentDepth + 1;
            }

            // 检查是否应该爬取该链接
            if (shouldCrawl(link))
            {
                m_urlQueue.enqueue(link);
            }
        }
    }

    void Crawler::requestNextUrl()
    {
        // 检查是否有URL可爬取
        if (m_urlQueue.isEmpty() || m_status != CrawlerStatus::Running)
        {
            return;
        }

        // 检查是否达到最大页面数限制
        if (m_config.maxPages > 0 && m_crawledUrls.size() >= m_config.maxPages)
        {
            m_status = CrawlerStatus::Completed;
            emit statusChanged(m_status);
            emit crawlingCompleted();
            INFOLOG("Crawler completed, reached maximum pages limit: {}", m_config.maxPages);
            return;
        }

        // 获取下一个URL
        QString url = m_urlQueue.dequeue();

        // 再次检查URL是否应该爬取
        if (!shouldCrawl(url))
        {
            // 继续请求下一个URL
            requestNextUrl();
            return;
        }

        // 添加到待处理集合
        m_pendingUrls.insert(url);

        // 创建网络请求
        QNetworkRequest request(url);
        m_networkManager->get(request);
    }

} // namespace IntelliSearch