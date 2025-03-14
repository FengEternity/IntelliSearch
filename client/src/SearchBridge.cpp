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
    , crawlerManager(std::make_unique<CrawlerManager>())
    , currentTurnNumber(0)
{
    if (!dbManager) {
        CRITICALLOG("Failed to create database manager");
        throw std::runtime_error("Database manager creation failed");
    }

    if (!dbManager->initialize()) {
        CRITICALLOG("Failed to initialize database");
        throw std::runtime_error("Database initialization failed");
    }

    connect(&searchWatcher, &QFutureWatcher<QString>::finished, 
            this, &SearchBridge::handleSearchComplete);
            
    // 连接爬虫管理器信号
    connect(crawlerManager.get(), &CrawlerManager::crawlingCompleted,
            this, &SearchBridge::crawlingCompleted);
    connect(crawlerManager.get(), &CrawlerManager::errorOccurred,
            this, &SearchBridge::crawlingError);
    connect(crawlerManager.get(), &CrawlerManager::progressChanged,
            this, &SearchBridge::crawlingProgress);

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
            auto searchResult = intentParser->search(intentParserResult["query"]);

            // 合并意图解析结果和搜索结果
            nlohmann::json combinedResult;
            combinedResult["intent_parser"] = intentParserResult;
            combinedResult["search_result"] = searchResult;

            // 将结果转换为 JSON 字符串
            QString jsonString = QString::fromStdString(combinedResult.dump());
            
            // 只有在搜索成功时才保存对话记录
            if (!searchResult.empty() && !currentSessionId.isEmpty()) {
                // 增加对话轮次
                currentTurnNumber++;
                
                // 保存对话记录
                if (!dbManager->addDialogueRecord(
                    currentSessionId,
                    lastQuery,
                    intentParserResult.contains("intent") ? intentParserResult["intent"].get<std::string>() : "",
                    intentParserResult.contains("query") ? QString::fromStdString(intentParserResult["query"].get<std::string>()) : lastQuery,
                    QString::fromStdString(searchResult.dump()),
                    currentTurnNumber
                )) {
                    WARNLOG("Failed to save dialogue record");
                } else {
                    emit sessionUpdated(currentSessionId);
                    emit sessionHistoryChanged();
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

    // 在搜索完成后更新会话历史
    emit sessionHistoryChanged();
}

QString SearchBridge::startNewSession() {
    INFOLOG("Creating new chat session");
    QString sessionId = dbManager->createSession();
    if (!sessionId.isEmpty()) {
        emit sessionCreated(sessionId);
        emit sessionHistoryChanged();  // 添加这行，确保在创建新会话时更新历史记录
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

/*
 * Summary: 开始爬取URL
 * Parameters:
 *   const QStringList& urls - 要爬取的URL列表
 * Return: void
 * Description: 调用爬虫管理器开始爬取URL
 */
void SearchBridge::startCrawling(const QStringList& urls) {
    if (urls.isEmpty()) {
        WARNLOG("No URLs provided for crawling");
        return;
    }
    
    INFOLOG("Starting crawling with {} URLs", urls.size());
    crawlerManager->startCrawling(urls);
    emit crawlingStarted();
}

/*
 * Summary: 停止爬取
 * Parameters: 无
 * Return: void
 * Description: 调用爬虫管理器停止爬取
 */
void SearchBridge::stopCrawling() {
    INFOLOG("Stopping crawling");
    crawlerManager->stopCrawling();
}

} // namespace IntelliSearch