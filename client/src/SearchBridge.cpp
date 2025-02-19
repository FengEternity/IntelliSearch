/*
 * Author: Montee
 * CreateDate: 2025-01-30
 * UpdateDate: 2025-02-09
 * UpdateReason: 新增删除搜索历史记录功能
 * Description: 搜索桥接类的实现，用于处理搜索请求和管理搜索历史
 */

#include "SearchBridge.h"
#include "../../log/Logger.h"
#include <QDebug>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <nlohmann/json.hpp>

namespace IntelliSearch {

/*
 * Summary: SearchBridge类构造函数
 * Parameters:
 *   QObject* parent - 父对象指针
 * Description: 初始化搜索桥接类，设置数据库管理器和异步搜索完成的信号处理
 */
SearchBridge::SearchBridge(QObject* parent)
    : QObject(parent)
    , intentParser(std::make_unique<IntentParser>())
    , dbManager(DatabaseManagerFactory::createDatabaseManager())
    , currentTurnNumber(0)
{
    // 1. 检查数据库管理器创建是否成功
    if (!dbManager) {
        CRITICALLOG("Failed to create database manager");
        throw std::runtime_error("Database manager creation failed");
    }

    // 2. 初始化数据库
    if (!dbManager->initialize()) {
        CRITICALLOG("Failed to initialize database");
        throw std::runtime_error("Database initialization failed");
    }

    // 3. 连接信号
    connect(&searchWatcher, &QFutureWatcher<QString>::finished, 
            this, &SearchBridge::handleSearchComplete);

    // 4. 创建初始会话
    QString sessionId = startNewSession();
    if (!sessionId.isEmpty()) {
        setCurrentSession(sessionId);
        INFOLOG("Created and set initial session: {}", sessionId.toStdString());
    } else {
        CRITICALLOG("Failed to create initial session");
        throw std::runtime_error("Initial session creation failed");
    }

    INFOLOG("SearchBridge initialization completed");
}

SearchBridge::~SearchBridge() = default;

/*
 * Summary: 处理搜索请求
 * Parameters:
 *   const QString& query - 搜索查询字符串
 * Return: void
 * Description: 异步处理搜索请求，解析搜索意图并返回结果
 */
void SearchBridge::handleSearch(const QString& query) {
    INFOLOG("Received search request: {}", query.toStdString());
    lastQuery = query;

    // 如果没有活动会话，创建新会话
    if (currentSessionId.isEmpty()) {
        QString sessionId = startNewSession();
        if (!sessionId.isEmpty()) {
            setCurrentSession(sessionId);
            INFOLOG("Created new session for search: {}", sessionId.toStdString());
        }
    }

    emit searchingChanged();
    QFuture<QString> future = QtConcurrent::run([this, query]() {
        try {
            DEBUGLOG("Starting async search for query: {}", query.toStdString());
            
            std::string stdQuery = query.toStdString();
            auto intentParserResult = intentParser->parseSearchIntent(stdQuery);
            auto searchResult = intentParser->bochaSearch(intentParserResult["query"]);

            // 合并意图解析结果和搜索结果
            nlohmann::json combinedResult;
            combinedResult["intent_parser"] = intentParserResult;
            combinedResult["search_result"] = searchResult;

            // 将结果转换为 JSON 字符串
            QString jsonString = QString::fromStdString(combinedResult.dump());
            
            // 获取当前活动的会话ID（这里需要添加一个成员变量来跟踪当前会话）
            if (!currentSessionId.isEmpty()) {
                // 增加对话轮次
                currentTurnNumber++;
                
                // 保存对话记录
                if (!dbManager->addDialogueRecord(
                    currentSessionId,
                    lastQuery,
                    intentParserResult.contains("intent") ? intentParserResult["intent"].get<std::string>() : "",
                    intentParserResult.contains("query") ? QString::fromStdString(intentParserResult["query"].get<std::string>()) : lastQuery,
                    QString::fromStdString(searchResult.dump()),
                    currentTurnNumber  // 使用累加的轮次号
                )) {
                    WARNLOG("Failed to save dialogue record");
                } else {
                    emit sessionUpdated(currentSessionId);
                }
            }
            
            DEBUGLOG("Search completed successfully");
            return jsonString;
            
        } catch (const std::exception& e) {
            ERRORLOG("Search failed: {}", e.what());
            return QString("{\"error\":\"%1\"}").arg(e.what());
        }
    });
    
    searchWatcher.setFuture(future);
}

/*
 * Summary: 处理搜索完成事件
 * Parameters: 无
 * Return: void
 * Description: 处理搜索完成事件，保存搜索记录到数据库并发出信号
 */
void SearchBridge::handleSearchComplete() {
    emit searchingChanged();
    try {
        QString result = searchWatcher.result();
        auto jsonResult = nlohmann::json::parse(result.toStdString());
        DEBUGLOG("Search results parsed: {}", jsonResult.dump().c_str());
        
        // 获取意图解析结果和搜索结果
        auto intentResult = jsonResult["intent_parser"];
        auto searchResult = jsonResult["search_result"];
        
        // 直接发送搜索结果
        emit searchResultsReady(QString::fromStdString(searchResult.dump()));
        
    } catch (const std::exception& e) {
        ERRORLOG("Error processing search results: {}", e.what());
        emit searchResultsReady(QString("{\"error\":\"搜索出错: %1\"}").arg(e.what()));
    }
}

QString SearchBridge::startNewSession() {
    INFOLOG("Creating new chat session");
    QString sessionId = dbManager->createSession();
    if (!sessionId.isEmpty()) {
        emit sessionCreated(sessionId);
        INFOLOG("Created new session with ID: {}", sessionId.toStdString());
    } else {
        ERRORLOG("Failed to create new session");
    }
    return sessionId;
}

QVariantList SearchBridge::getSessionsList(int limit) {
    DEBUGLOG("Retrieving sessions list with limit: {}", limit);
    QVariantList sessionsList;
    
    auto sessions = dbManager->getSessionHistory(limit);
    for (const auto& session : sessions) {
        QVariantMap sessionMap;
        sessionMap["id"] = session.first;  // session_id
        // 将 session.second 中的所有键值对添加到 sessionMap
        for (auto it = session.second.begin(); it != session.second.end(); ++it) {
            sessionMap[it.key()] = it.value();
        }
        sessionsList.append(sessionMap);
    }
    
    return sessionsList;
}

QVariantList SearchBridge::getSessionDialogues(const QString& sessionId) {
    DEBUGLOG("Retrieving dialogues for session: {}", sessionId.toStdString());
    QVariantList dialoguesList;
    
    auto dialogues = dbManager->getDialogueHistory(sessionId);
    for (const auto& dialogue : dialogues) {
        dialoguesList.append(dialogue);
    }
    
    return dialoguesList;
}

void SearchBridge::setCurrentSession(const QString& sessionId) {
    if (currentSessionId != sessionId) {
        currentSessionId = sessionId;
        currentTurnNumber = 0;  // 重置对话轮次
        INFOLOG("Set current session to: {}", sessionId.toStdString());
    }
}
} // namespace IntelliSearch