#ifndef INTELLISEARCH_CRAWLERMANAGER_H
#define INTELLISEARCH_CRAWLERMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVariantList>
#include <memory>
#include <queue>
#include <mutex>
#include <QThreadPool>
#include <QRunnable>
#include "PythonCrawlerBridge.h"

namespace IntelliSearch
{

    // 爬虫管理器类，用于管理爬虫实例和与前端交互
    class CrawlerManager : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(bool isCrawling READ isCrawling NOTIFY crawlingStatusChanged)
        Q_PROPERTY(int crawledCount READ getCrawledCount NOTIFY progressChanged)
        Q_PROPERTY(int totalCount READ getTotalCount NOTIFY progressChanged)

    public:
        explicit CrawlerManager(QObject *parent = nullptr);
        ~CrawlerManager();

        // 检查是否正在爬取
        bool isCrawling() const;

        // 获取已爬取的URL数量
        int getCrawledCount() const;

        // 获取总URL数量（已爬取+待爬取）
        int getTotalCount() const;

    public slots:
        // 开始爬取单个URL
        Q_INVOKABLE void startCrawling(const QString &url);

        // 开始爬取多个URL
        Q_INVOKABLE void startCrawling(const QStringList &urls);

        // 暂停爬取
        Q_INVOKABLE void pauseCrawling();

        // 恢复爬取
        Q_INVOKABLE void resumeCrawling();

        // 停止爬取
        Q_INVOKABLE void stopCrawling();

        // 设置爬虫配置
        Q_INVOKABLE void setMaxDepth(int depth);
        Q_INVOKABLE void setMaxPages(int pages);
        Q_INVOKABLE void setRequestDelay(int delay);
        Q_INVOKABLE void setFollowExternalLinks(bool follow);
        Q_INVOKABLE void setAllowedDomains(const QStringList &domains);
        Q_INVOKABLE void setUrlFilters(const QStringList &filters);
        Q_INVOKABLE void setUseDynamicCrawling(bool useDynamic);

        // 获取爬取结果
        Q_INVOKABLE QVariantList getCrawlResults(int limit = 100) const;

    signals:
        // 爬取进度信号
        void progressChanged(int crawled, int total);

        // 爬取状态变化信号
        void crawlingStatusChanged(bool isCrawling);

        // 爬取结果信号
        void resultReady(const QVariantMap &result);

        // 爬取完成信号
        void crawlingCompleted();

        // 爬取错误信号
        void errorOccurred(const QString &errorMessage);

    private slots:
        // 处理爬虫进度变化
        void handleProgressChanged(int crawled, int total);

        // 处理爬虫状态变化
        void handleStatusChanged(PythonCrawlerStatus status);

        // 处理爬虫结果
        void handleResultReady(const CrawlResult &result);

        // 处理爬虫完成
        void handleCrawlingCompleted();

        // 处理爬虫错误
        void handleErrorOccurred(const QString &errorMessage);

    private:
        // 爬虫任务类
        class CrawlerTask : public QRunnable {
        public:
            CrawlerTask(const QString& url, PythonCrawlerBridge* crawler);
            void run() override;
        private:
            QString m_url;
            PythonCrawlerBridge* m_crawler;
        };

        std::unique_ptr<PythonCrawlerBridge> m_pythonCrawler; // Python爬虫实例
        QList<CrawlResult> m_results;       // 爬取结果列表
        bool m_isCrawling;                  // 是否正在爬取
        int m_crawledCount;                 // 已爬取的URL数量
        int m_totalCount;                   // 总URL数量

        QThreadPool m_threadPool;           // 线程池
        std::queue<QString> m_taskQueue;    // 任务队列
        std::mutex m_mutex;                 // 互斥锁
        int m_maxThreads;                   // 最大线程数
    };

} // namespace IntelliSearch

#endif // INTELLISEARCH_CRAWLERMANAGER_H