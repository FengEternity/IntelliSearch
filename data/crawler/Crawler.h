#ifndef INTELLISEARCH_CRAWLER_H
#define INTELLISEARCH_CRAWLER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QQueue>
#include <QSet>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QRegularExpression>
#include <memory>
#include "HtmlParser.h"

namespace IntelliSearch
{

    // 爬虫状态枚举
    enum class CrawlerStatus
    {
        Idle,      // 空闲状态
        Running,   // 运行中
        Paused,    // 暂停
        Completed, // 完成
        Error      // 错误
    };

    // 使用HtmlParser.h中定义的CrawlResult结构体

    // 爬虫配置结构体
    struct CrawlerConfig
    {
        int maxDepth = 2;                 // 最大爬取深度
        int maxPages = 100;               // 最大爬取页面数
        int requestDelay = 1000;          // 请求延迟(毫秒)
        bool followExternalLinks = false; // 是否跟随外部链接
        QStringList allowedDomains;       // 允许的域名列表
        QStringList urlFilters;           // URL过滤规则
        QStringList contentFilters;       // 内容过滤规则
    };

    // 爬虫基类
    class Crawler : public QObject
    {
        Q_OBJECT

    public:
        explicit Crawler(QObject *parent = nullptr);
        virtual ~Crawler();

        // 开始爬取指定URL
        virtual void startCrawling(const QString &url);

        // 开始爬取多个URL
        virtual void startCrawling(const QStringList &urls);

        // 暂停爬取
        virtual void pauseCrawling();

        // 恢复爬取
        virtual void resumeCrawling();

        // 停止爬取
        virtual void stopCrawling();

        // 设置爬虫配置
        virtual void setConfig(const CrawlerConfig &config);

        // 获取爬虫配置
        virtual CrawlerConfig getConfig() const;

        // 获取爬虫状态
        virtual CrawlerStatus getStatus() const;

        // 获取已爬取的URL数量
        virtual int getCrawledCount() const;

    signals:
        // 爬取进度信号
        void progressChanged(int crawled, int total);

        // 爬取状态变化信号
        void statusChanged(CrawlerStatus status);

        // 爬取结果信号
        void resultReady(const CrawlResult &result);

        // 爬取完成信号
        void crawlingCompleted();

        // 爬取错误信号
        void errorOccurred(const QString &errorMessage);

    protected:
        // 处理网络响应
        virtual void handleNetworkReply(QNetworkReply *reply);

        // 检查URL是否应该被爬取
        virtual bool shouldCrawl(const QString &url);

        // 处理爬取结果
        virtual void processResult(const CrawlResult &result);

        // 请求下一个URL
        virtual void requestNextUrl();

    protected:
        QNetworkAccessManager *m_networkManager; // 网络访问管理器
        QQueue<QString> m_urlQueue;              // 待爬取URL队列
        QSet<QString> m_crawledUrls;             // 已爬取URL集合
        QSet<QString> m_pendingUrls;             // 正在爬取的URL集合
        CrawlerStatus m_status;                  // 爬虫状态
        CrawlerConfig m_config;                  // 爬虫配置
        QTimer *m_requestTimer;                  // 请求定时器
        int m_currentDepth;                      // 当前爬取深度
        QMap<QString, int> m_urlDepthMap;        // URL深度映射
        HtmlParser m_htmlParser;                 // HTML解析器
    };

} // namespace IntelliSearch

#endif // INTELLISEARCH_CRAWLER_H