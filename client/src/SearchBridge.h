#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <memory>
#include "core/engine/IntentParser.h"
#include "../../data/database/DatabaseManager.h"
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

namespace IntelliSearch {

class SearchBridge : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isSearching READ isSearching NOTIFY searchingChanged)
    Q_PROPERTY(QVariantList sessionHistory READ getSessionsList NOTIFY sessionHistoryChanged)

public:
    explicit SearchBridge(QObject* parent = nullptr);
    ~SearchBridge();

public slots:
    void handleSearch(const QString& query);
    
    // 会话管理相关方法
    QString startNewSession();  // 开始新会话
    QVariantList getSessionsList(int limit = 10);  // 获取会话列表
    QVariantList getSessionDialogues(const QString& sessionId);  // 获取特定会话的对话历史
    void setCurrentSession(const QString& sessionId);  // 设置当前活动会话
    
    // 创建新会话并自动切换到该会话
    Q_INVOKABLE QString createAndSwitchToNewSession() {
        QString sessionId = startNewSession();
        if (!sessionId.isEmpty()) {
            setCurrentSession(sessionId);
        }
        return sessionId;
    }

signals:
    void searchResultsReady(const QString& results); // 搜索结果就绪
    void searchingChanged();   // 搜索状态改变
    void searchStatusChanged(const QString &status); // 搜索状态改变
    void sessionCreated(const QString& sessionId); // 会话创建
    void sessionUpdated(const QString& sessionId); // 会话更新
    void sessionHistoryChanged();  // 会话历史改变

private slots:
    void handleSearchComplete();

private:
    std::unique_ptr<IntentParser> intentParser;
    std::shared_ptr<IDatabaseManager> dbManager;
    QFutureWatcher<QString> searchWatcher;
    QString currentSessionId;
    QString lastQuery;
    int currentTurnNumber;

    bool isSearching() const { return searchWatcher.isRunning(); }
    void updateSessionHistory();
};

} // namespace IntelliSearch