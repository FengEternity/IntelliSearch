#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <memory>
#include "core/engine/IntentParser.h"
#include "../../data/database/DatabaseManager.h"
#include "../../data/crawler/CrawlerManager.h"
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

namespace IntelliSearch
{

    class SearchBridge : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(bool isSearching READ isSearching NOTIFY searchingChanged)
        Q_PROPERTY(QVariantList sessionHistory READ getSessionsList NOTIFY sessionHistoryChanged)
        Q_PROPERTY(CrawlerManager *crawlerManager READ getCrawlerManager CONSTANT)

    public:
        explicit SearchBridge(QObject *parent = nullptr);
        ~SearchBridge();

        // 获取爬虫管理器实例
        CrawlerManager *getCrawlerManager() const { return crawlerManager.get(); }

    public slots:
        void handleSearch(const QString &query);

        // 会话管理相关方法
        QString startNewSession();                                  // 开始新会话
        QVariantList getSessionsList(int limit = 10);               // 获取会话列表
        QVariantList getSessionDialogues(const QString &sessionId); // 获取特定会话的对话历史
        void setCurrentSession(const QString &sessionId);           // 设置当前活动会话

        // 创建新会话并自动切换到该会话
        Q_INVOKABLE QString createAndSwitchToNewSession()
        {
            QString sessionId = startNewSession();
            if (!sessionId.isEmpty())
            {
                setCurrentSession(sessionId);
            }
            return sessionId;
        }

        // 爬虫相关方法
        Q_INVOKABLE void startCrawling(const QStringList &urls);
        Q_INVOKABLE void stopCrawling();

    signals:
        void searchResultsReady(const QString &results); // 搜索结果就绪
        void searchingChanged();                         // 搜索状态改变
        void searchStatusChanged(const QString &status); // 搜索状态改变
        void sessionCreated(const QString &sessionId);   // 会话创建
        void sessionUpdated(const QString &sessionId);   // 会话更新
        void sessionHistoryChanged();                    // 会话历史改变

        // 爬虫相关信号
        void crawlingStarted();                          // 爬取开始
        void crawlingCompleted();                        // 爬取完成
        void crawlingError(const QString &errorMessage); // 爬取错误
        void crawlingProgress(int crawled, int total);   // 爬取进度

    private slots:
        void handleSearchComplete();

    private:
        std::unique_ptr<IntentParser> intentParser;
        std::shared_ptr<IDatabaseManager> dbManager;
        std::unique_ptr<CrawlerManager> crawlerManager; // 爬虫管理器
        QFutureWatcher<QString> searchWatcher;
        QString currentSessionId;
        QString lastQuery;
        int currentTurnNumber;

        bool isSearching() const { return searchWatcher.isRunning(); }
        void updateSessionHistory();
    };

} // namespace IntelliSearch