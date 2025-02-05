#ifndef INTELLISEARCH_DATABASEMANAGER_H
#define INTELLISEARCH_DATABASEMANAGER_H

#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <memory>

namespace IntelliSearch {

// 抽象数据库管理接口
class IDatabaseManager {
public:
    virtual ~IDatabaseManager() = default;
    
    // 初始化数据库连接和表结构
    virtual bool initialize() = 0;
    
    // 添加搜索历史记录
    virtual bool addSearchHistory(const QString& query, const QString& result) = 0;
    
    // 获取搜索历史记录
    virtual QVector<QPair<QString, QString>> getSearchHistory(int limit = 10) = 0;
    
    // 清除搜索历史记录
    virtual bool clearSearchHistory() = 0;
};

// SQLite实现类
class SQLiteDatabaseManager : public QObject, public IDatabaseManager {
    Q_OBJECT

public:
    explicit SQLiteDatabaseManager(QObject* parent = nullptr);
    ~SQLiteDatabaseManager() override;

    bool initialize() override;
    bool addSearchHistory(const QString& query, const QString& result) override;
    QVector<QPair<QString, QString>> getSearchHistory(int limit = 10) override;
    bool clearSearchHistory() override;

private:
    QSqlDatabase db;
    const QString DATABASE_NAME = "intellisearch.db";
    const QString TABLE_NAME = "search_history";
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