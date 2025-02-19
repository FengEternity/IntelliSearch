#ifndef INTELLISEARCH_DATABASEMANAGER_H
#define INTELLISEARCH_DATABASEMANAGER_H

#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QVariantMap>
#include <memory>

namespace IntelliSearch {

// 抽象数据库管理接口
class IDatabaseManager {
public:
    virtual ~IDatabaseManager() = default;
    
    // 初始化数据库连接和表结构
    virtual bool initialize() = 0;
    
    // 新增会话相关方法
    virtual QString createSession() = 0;  // 创建新会话，返回会话ID
    virtual bool addDialogueRecord(
        const QString& sessionId,
        const QString& user_query,
        std::basic_string<char> intent_type,
        const QString& intent_result,
        const QString& search_result,
        int turn_number) = 0;  // 添加对话记录
        
    virtual QVector<QPair<QString, QVariantMap>> getSessionHistory(int limit = 10) = 0;  // 获取会话历史
    virtual QVector<QVariantMap> getDialogueHistory(const QString& sessionId) = 0;  // 获取特定会话的对话历史
};

// SQLite实现类
class SQLiteDatabaseManager : public QObject, public IDatabaseManager {
    Q_OBJECT

public:
    explicit SQLiteDatabaseManager(QObject* parent = nullptr);
    ~SQLiteDatabaseManager() override;

    bool initialize() override;
    QString createSession() override;
    bool addDialogueRecord(
        const QString& sessionId,
        const QString& user_query,
        std::basic_string<char> intent_type,
        const QString& intent_result,
        const QString& search_result,
        int turn_number) override;
    QVector<QPair<QString, QVariantMap>> getSessionHistory(int limit = 10) override;
    QVector<QVariantMap> getDialogueHistory(const QString& sessionId) override;

private:
    QSqlDatabase db;
    const QString DATABASE_NAME = "intellisearch.db";
    const QString SESSIONS_TABLE = "dialogue_sessions";
    const QString DIALOGUES_TABLE = "dialogue_records";
};

// 数据库管理器工厂
class DatabaseManagerFactory {
public:
    static std::unique_ptr<IDatabaseManager> createDatabaseManager(const QString& type = "sqlite") {
        if (type.toLower() == "sqlite") {
            return std::make_unique<SQLiteDatabaseManager>();
        }
        // 未来可以在这里添加其他数据库实现
        return nullptr;
    }
};

} // namespace IntelliSearch

#endif // INTELLISEARCH_DATABASEMANAGER_H