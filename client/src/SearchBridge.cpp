#include "SearchBridge.h"
#include "../../log/Logger.h"
#include <QDebug>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

namespace IntelliSearch {

SearchBridge::SearchBridge(QObject* parent)
    : QObject(parent)
    , intentParser(std::make_unique<IntentParser>())
    , dbManager(DatabaseManagerFactory::createDatabaseManager())
{
    if (!dbManager || !dbManager->initialize()) {
        ERRORLOG("Failed to initialize database manager");
        throw std::runtime_error("Database initialization failed");
    }

    // 连接异步完成信号
    connect(&searchWatcher, &QFutureWatcher<QString>::finished, this, [this]() {
        try {
            QString result = searchWatcher.result();
            
            // 保存搜索记录到数据库
            if (!dbManager->addSearchHistory(lastQuery, result)) {
                WARNLOG("Failed to save search history");
            } else {
                emit searchHistoryChanged();
            }
            
            INFOLOG("Async search completed, sending results");
            emit searchResultsReady(result);
        } catch (const std::exception& e) {
            ERRORLOG("Error processing search results: {}", e.what());
            emit searchResultsReady(QString("{\"error\":\"%1\"}").arg(e.what()));
        }
    });

    INFOLOG("SearchBridge initialization completed");
}

SearchBridge::~SearchBridge() = default;

void SearchBridge::handleSearch(const QString& query) {
    INFOLOG("Received search request: {}", query.toStdString());
    lastQuery = query;  // 保存查询字符串供后续使用

    emit searchingChanged();  // 开始搜索时发出信号
    QFuture<QString> future = QtConcurrent::run([this, query]() {
        try {
            DEBUGLOG("Starting async search for query: {}", query.toStdString());
            
            std::string stdQuery = query.toStdString();
            auto result = intentParser->parseSearchIntent(stdQuery);
            
            // 将结果转换为 JSON 字符串
            QString jsonString = QString::fromStdString(result.dump());
            DEBUGLOG("Search completed successfully");
            
            return jsonString;
        } catch (const std::exception& e) {
            ERRORLOG("Search failed: {}", e.what());
            return QString("{\"error\":\"%1\"}").arg(e.what());
        }
    });

    connect(&searchWatcher, &QFutureWatcher<QString>::finished, this, [this]() {
        emit searchingChanged();  // 搜索完成时发出信号
        try {
            QString result = searchWatcher.result();
            
            // 保存搜索记录到数据库
            if (!dbManager->addSearchHistory(lastQuery, result)) {
                WARNLOG("Failed to save search history");
            } else {
                emit searchHistoryChanged();
            }
            
            INFOLOG("Async search completed, sending results");
            emit searchResultsReady(result);
        } catch (const std::exception& e) {
            ERRORLOG("Error processing search results: {}", e.what());
            emit searchResultsReady(QString("{\"error\":\"%1\"}").arg(e.what()));
        }
    });
    
    searchWatcher.setFuture(future);
}

QVariantList SearchBridge::getSearchHistory() {
    INFOLOG("Getting search history records");
    QVariantList historyList;
    auto history = dbManager->getSearchHistory(10); // 获取最近10条记录
    
    if (history.isEmpty()) {
        DEBUGLOG("No search history records found");
        return historyList;
    }
    
    for (const auto& record : history) {
        if (!record.first.isEmpty()) {
            QVariantMap item;
            item["query"] = record.first;
            historyList.append(item);
            
            // INFOLOG("History record - Query: {}", record.first.toStdString());
        }
    }
    
    INFOLOG("Retrieved {} search history records", historyList.size());
    return historyList;
}

} // namespace IntelliSearch