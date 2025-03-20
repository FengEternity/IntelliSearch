#ifndef INTELLISEARCH_PYTHONCRAWLERBRIDGE_H
#define INTELLISEARCH_PYTHONCRAWLERBRIDGE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QVariantMap>
#include <QVariantList>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QDateTime>
#include <memory>
#include "CrawlResult.h"

namespace IntelliSearch
{

    // Python爬虫状态枚举
    enum class PythonCrawlerStatus
    {
        Idle,      // 空闲状态
        Running,   // 运行中
        Paused,    // 暂停
        Completed, // 完成
        Error      // 错误
    };

    // Python爬虫配置结构体
    struct PythonCrawlerConfig
    {
        int maxDepth = 2;                 // 最大爬取深度
        int maxPages = 10;               // 最大爬取页面数
        int requestDelay = 1000;          // 请求延迟(毫秒)
        bool followExternalLinks = false; // 是否跟随外部链接
        bool useDynamicCrawling = true;  // 是否使用动态爬取
        int pageLoadTimeout = 30000;      // 页面加载超时时间（毫秒）
        QStringList allowedDomains;       // 允许的域名列表
        QStringList urlFilters;           // URL过滤规则
        QString pythonPath;               // Python解释器路径
        QString crawlerScript;            // 爬虫脚本路径
        QString configPath;               // 配置文件路径
        QString outputDir;                // 输出目录
    };

    // Python爬虫桥接类，负责与Python爬虫进程通信
    class PythonCrawlerBridge : public QObject
    {
        Q_OBJECT

    public:
        explicit PythonCrawlerBridge(QObject *parent = nullptr);
        virtual ~PythonCrawlerBridge();

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
        virtual void setConfig(const PythonCrawlerConfig &config);

        // 获取爬虫配置
        virtual PythonCrawlerConfig getConfig() const;

        // 获取爬虫状态
        virtual PythonCrawlerStatus getStatus() const;

        // 获取已爬取的URL数量
        virtual int getCrawledCount() const;

        // 获取总URL数量
        virtual int getTotalCount() const;

    signals:
        // 爬取进度信号
        void progressChanged(int crawled, int total);

        // 爬取状态变化信号
        void statusChanged(PythonCrawlerStatus status);

        // 爬取结果信号
        void resultReady(const CrawlResult &result);

        // 爬取完成信号
        void crawlingCompleted();

        // 爬取错误信号
        void errorOccurred(const QString &errorMessage);

    private slots:
        // 处理Python进程输出
        void handleProcessOutput();

        // 处理Python进程错误输出
        void handleProcessError();

        // 处理Python进程完成
        void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

        // 检查爬取结果文件
        void checkResultsFile();

    private:
        // 生成配置文件
        bool generateConfigFile();

        // 解析爬取结果
        void parseResults(const QString &resultsFile);

        // 转换爬取结果为CrawlResult
        CrawlResult convertToCrawlResult(const QJsonObject &jsonResult);

        // 发送命令到Python进程
        bool sendCommand(const QString &command);

        std::unique_ptr<QProcess> m_process;       // Python进程
        PythonCrawlerConfig m_config;              // 爬虫配置
        PythonCrawlerStatus m_status;              // 爬虫状态
        QList<CrawlResult> m_results;              // 爬取结果列表
        int m_crawledCount;                        // 已爬取的URL数量
        int m_totalCount;                          // 总URL数量
        QString m_currentResultsFile;              // 当前结果文件路径
        QTimer m_resultsCheckTimer;                // 结果检查定时器
    };

} // namespace IntelliSearch

#endif // INTELLISEARCH_PYTHONCRAWLERBRIDGE_H