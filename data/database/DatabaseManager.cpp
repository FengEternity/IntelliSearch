#include "DatabaseManager.h"
#include <QDir>
#include <QStandardPaths>
#include "../../log/Logger.h"

namespace IntelliSearch {

SQLiteDatabaseManager::SQLiteDatabaseManager(QObject* parent) : QObject(parent) {
    try {
        // 在应用数据目录下创建数据库文件
        // QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        // 在构建目录下创建数据库文件
        QString dataPath = QDir::currentPath();
        DEBUGLOG("Current working directory: {}", dataPath.toStdString());

        QDir dir(dataPath);
        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                throw std::runtime_error("Failed to create database directory");
            }
            INFOLOG("Created database directory at: {}", dataPath.toStdString());
        }

        // 检查目录权限
        QFileInfo dirInfo(dataPath);
        if (!dirInfo.isWritable()) {
            throw std::runtime_error("Database directory is not writable");
        }
        DEBUGLOG("Database directory permissions check passed");
        
        // 初始化数据库连接
        if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
            throw std::runtime_error("SQLite driver is not available");
        }
        
        db = QSqlDatabase::addDatabase("QSQLITE");
        QString dbPath = dir.filePath(DATABASE_NAME);
        db.setDatabaseName(dbPath);
        
        // 检查数据库文件权限
        QFileInfo dbInfo(dbPath);
        if (dbInfo.exists() && !dbInfo.isWritable()) {
            throw std::runtime_error("Database file exists but is not writable");
        }
        
        INFOLOG("Database initialized at: {}", dbPath.toStdString());
    } catch (const std::exception& e) {
        ERRORLOG("Database initialization error: {}", e.what());
        throw; // 重新抛出异常以便上层处理
    }
}

SQLiteDatabaseManager::~SQLiteDatabaseManager() {
    if (db.isOpen()) {
        db.close();
        DEBUGLOG("Database connection closed");
    }
}

bool SQLiteDatabaseManager::initialize() {
    if (!db.open()) {
        ERRORLOG("Failed to open database: {}", db.lastError().text().toStdString());
        return false;
    }
    INFOLOG("Database connection opened successfully");

    // 创建搜索历史表
    QSqlQuery query;
    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS " + TABLE_NAME + " ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_query TEXT NOT NULL,"
        "intent_type TEXT NOT NULL,"
        "intent_result TEXT NOT NULL,"
        "search_result TEXT NOT NULL,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ")"
    );

    if (!success) {
        ERRORLOG("Failed to create table {}: {}", TABLE_NAME.toStdString(), query.lastError().text().toStdString());
    } else {
        INFOLOG("Table {} created or already exists", TABLE_NAME.toStdString());
    }
    return success;
}

bool SQLiteDatabaseManager::addSearchHistory(
    const QString& user_query = "",
    std::basic_string<char> intent_type = "",
    const QString& intent_result = "",
    const QString& search_result= "")
{
    QSqlQuery sqlQuery;
    sqlQuery.prepare("INSERT INTO " + TABLE_NAME + " (user_query, intent_type, intent_result, search_result) "
                    "VALUES (?, ?, ?, ?)");
    sqlQuery.addBindValue(user_query);
    sqlQuery.addBindValue(QString::fromStdString(intent_type));
    sqlQuery.addBindValue(intent_result);
    sqlQuery.addBindValue(search_result);
    bool success = sqlQuery.exec();

    if (!success) {
        ERRORLOG("Failed to add search history: {}", sqlQuery.lastError().text().toStdString());
    } else {
        DEBUGLOG("Added search history - Query: {}", user_query.toStdString());
    }
    return success;
}

QVector<QPair<QString, QString>> SQLiteDatabaseManager::getSearchHistory(int limit) {
    QVector<QPair<QString, QString>> history;
    QSqlQuery query;
    
    query.prepare("SELECT id, user_query FROM " + TABLE_NAME + 
                 " ORDER BY timestamp DESC LIMIT ?");
    query.addBindValue(limit);
    
    if (!query.exec()) {
        ERRORLOG("Failed to retrieve search history: {}", query.lastError().text().toStdString());
        return history;
    }
    
    while (query.next()) {
        QString id = query.value(0).toString();
        QString userQuery = query.value(1).toString();
        history.append(qMakePair(id, userQuery));
    }
    
    DEBUGLOG("Retrieved {} search history records", history.size());
    return history;
}

bool SQLiteDatabaseManager::deleteSearchHistory(const QString& recordId) {
    INFOLOG("Deleting search history record with ID: {}", recordId.toStdString());
    QSqlQuery sqlQuery;
    sqlQuery.prepare("DELETE FROM " + TABLE_NAME + " WHERE id = ?");
    sqlQuery.addBindValue(recordId);
    bool success = sqlQuery.exec();
    if (!success) {
        ERRORLOG("Failed to delete search history: {}", sqlQuery.lastError().text().toStdString());
    } else {
        DEBUGLOG("Deleted search history record with ID: {}", recordId.toStdString());
    }
    return success;
}

bool SQLiteDatabaseManager::clearSearchHistory() {
    QSqlQuery query;
    bool success = query.exec("DELETE FROM " + TABLE_NAME);

    if (!success) {
        ERRORLOG("Failed to clear search history: {}", query.lastError().text().toStdString());
    } else {
        INFOLOG("Search history cleared successfully");
    }
    return success;
}

} // namespace IntelliSearch