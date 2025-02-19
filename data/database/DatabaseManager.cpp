#include "DatabaseManager.h"
#include <QDir>
#include <QStandardPaths>
#include "../../log/Logger.h"
#include <QUuid>

namespace IntelliSearch {

SQLiteDatabaseManager::SQLiteDatabaseManager(QObject* parent) : QObject(parent) {
    try {
        QString dataPath = QDir::currentPath();
        DEBUGLOG("Current working directory: {}", dataPath.toStdString());

        QDir dir(dataPath);
        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                throw std::runtime_error("Failed to create database directory");
            }
            INFOLOG("Created database directory at: {}", dataPath.toStdString());
        }

        // 1. 给数据库连接一个唯一的名称
        QString connectionName = "SQLiteConnection_" + QString::number(reinterpret_cast<qulonglong>(this));
        
        // 2. 检查驱动并创建连接
        if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
            CRITICALLOG("SQLite driver is not available");
            throw std::runtime_error("SQLite driver is not available");
        }
        
        // 3. 创建新的数据库连接
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        QString dbPath = dir.filePath(DATABASE_NAME);
        db.setDatabaseName(dbPath);
        
        INFOLOG("Database initialized at: {}", dbPath.toStdString());
        
    } catch (const std::exception& e) {
        CRITICALLOG("Database initialization error: {}", e.what());
        throw;
    }
}

SQLiteDatabaseManager::~SQLiteDatabaseManager() {
    QString connectionName = db.connectionName();
    if (db.isOpen()) {
        db.close();
    }
    db = QSqlDatabase(); // 清除连接
    QSqlDatabase::removeDatabase(connectionName); // 移除连接
    DEBUGLOG("Database connection {} closed and removed", connectionName.toStdString());
}

bool SQLiteDatabaseManager::initialize() {
    // 1. 首先确保数据库连接是打开的
    if (!db.isOpen() && !db.open()) {
        ERRORLOG("Failed to open database: {}", db.lastError().text().toStdString());
        return false;
    }

    // 2. 开始事务
    if (!db.transaction()) {
        ERRORLOG("Failed to start transaction: {}", db.lastError().text().toStdString());
        return false;
    }

    try {
        // 3. 创建会话表
        QSqlQuery query(db);  // 使用当前数据库连接
        bool success = query.exec(
            "CREATE TABLE IF NOT EXISTS " + SESSIONS_TABLE + " ("
            "session_id TEXT PRIMARY KEY,"
            "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "last_updated DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "title TEXT,"
            "status TEXT DEFAULT 'active'"
            ")"
        );

        if (!success) {
            ERRORLOG("Failed to create sessions table: {}", query.lastError().text().toStdString());
            db.rollback();
            return false;
        }

        // 4. 创建对话记录表
        success = query.exec(
            "CREATE TABLE IF NOT EXISTS " + DIALOGUES_TABLE + " ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "session_id TEXT NOT NULL,"
            "turn_number INTEGER NOT NULL,"
            "user_query TEXT NOT NULL,"
            "intent_type TEXT NOT NULL,"
            "intent_result TEXT NOT NULL,"
            "search_result TEXT NOT NULL,"
            "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "FOREIGN KEY(session_id) REFERENCES " + SESSIONS_TABLE + "(session_id)"
            ")"
        );

        if (!success) {
            ERRORLOG("Failed to create dialogues table: {}", query.lastError().text().toStdString());
            db.rollback();
            return false;
        }

        // 6. 提交事务
        if (!db.commit()) {
            ERRORLOG("Failed to commit transaction: {}", db.lastError().text().toStdString());
            db.rollback();
            return false;
        }

        INFOLOG("Database tables initialized successfully");
        return true;

    } catch (const std::exception& e) {
        ERRORLOG("Exception during database initialization: {}", e.what());
        db.rollback();
        return false;
    }
}

QString SQLiteDatabaseManager::createSession() {
    // 1. 检查数据库连接
    if (!db.isOpen() && !db.open()) {
        ERRORLOG("Database connection is not open");
        return QString();
    }

    QString sessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    
    // 2. 使用正确的数据库连接创建查询
    QSqlQuery query(db);  // 显式指定数据库连接
    query.prepare("INSERT INTO " + SESSIONS_TABLE + " (session_id) VALUES (?)");
    query.addBindValue(sessionId);
    
    if (!query.exec()) {
        ERRORLOG("Failed to create session: {}", query.lastError().text().toStdString());
        return QString();
    }
    
    return sessionId;
}

bool SQLiteDatabaseManager::addDialogueRecord(
    const QString& sessionId,
    const QString& user_query,
    std::basic_string<char> intent_type,
    const QString& intent_result,
    const QString& search_result,
    int turn_number = 0)
{
    if (!db.isOpen() && !db.open()) {
        ERRORLOG("Database connection is not open");
        return false;
    }

    QSqlQuery query(db);  // 显式指定数据库连接
    query.prepare("INSERT INTO " + DIALOGUES_TABLE + 
                 " (session_id, turn_number, user_query, intent_type, intent_result, search_result) "
                 "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(sessionId);
    query.addBindValue(turn_number);
    query.addBindValue(user_query);
    query.addBindValue(QString::fromStdString(intent_type));
    query.addBindValue(intent_result);
    query.addBindValue(search_result);
    
    bool success = query.exec();
    
    if (success) {
        // 更新会话的最后更新时间
        query.prepare("UPDATE " + SESSIONS_TABLE + 
                     " SET last_updated = CURRENT_TIMESTAMP, title = COALESCE(title, ?) "
                     "WHERE session_id = ?");
        query.addBindValue(user_query);  // 如果标题为空，使用第一条用户查询作为标题
        query.addBindValue(sessionId);
        success = query.exec();
    }

    if (!success) {
        ERRORLOG("Failed to add dialogue record: {}", query.lastError().text().toStdString());
    } else {
        DEBUGLOG("Added dialogue record - Session: {}, Turn: {}", sessionId.toStdString(), turn_number);
    }

    return success;
}

QVector<QPair<QString, QVariantMap>> SQLiteDatabaseManager::getSessionHistory(int limit) {
    QVector<QPair<QString, QVariantMap>> sessions;
    QSqlQuery query;
    
    query.prepare("SELECT session_id, title, created_at, last_updated, "
                 "(SELECT COUNT(*) FROM " + DIALOGUES_TABLE + " WHERE session_id = s.session_id) as message_count "
                 "FROM " + SESSIONS_TABLE + " s "
                 "ORDER BY last_updated DESC LIMIT ?");
    query.addBindValue(limit);
    
    if (query.exec()) {
        while (query.next()) {
            QVariantMap sessionInfo;
            QString sessionId = query.value(0).toString();
            sessionInfo["title"] = query.value(1).toString();
            sessionInfo["created_at"] = query.value(2).toString();
            sessionInfo["last_updated"] = query.value(3).toString();
            sessionInfo["message_count"] = query.value(4).toInt();
            
            sessions.append(qMakePair(sessionId, sessionInfo));
        }
    }
    
    return sessions;
}

QVector<QVariantMap> SQLiteDatabaseManager::getDialogueHistory(const QString& sessionId) {
    QVector<QVariantMap> dialogues;
    QSqlQuery query;
    
    query.prepare("SELECT * FROM " + DIALOGUES_TABLE + 
                 " WHERE session_id = ? ORDER BY turn_number ASC");
    query.addBindValue(sessionId);
    
    if (query.exec()) {
        while (query.next()) {
            QVariantMap dialogue;
            dialogue["turn_number"] = query.value("turn_number").toInt();
            dialogue["user_query"] = query.value("user_query").toString();
            dialogue["intent_type"] = query.value("intent_type").toString();
            dialogue["intent_result"] = query.value("intent_result").toString();
            dialogue["search_result"] = query.value("search_result").toString();
            dialogue["timestamp"] = query.value("timestamp").toString();
            
            dialogues.append(dialogue);
        }
    }
    
    return dialogues;
}

} // namespace IntelliSearch