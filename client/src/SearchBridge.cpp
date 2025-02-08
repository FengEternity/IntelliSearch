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
{
    if (!dbManager || !dbManager->initialize()) {
        ERRORLOG("Failed to initialize database manager");
        throw std::runtime_error("Database initialization failed");
    }

    // 在构造函数中连接信号
    connect(&searchWatcher, &QFutureWatcher<QString>::finished, this, &SearchBridge::handleSearchComplete);

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
    
    searchWatcher.setFuture(future);
}

/*
 * Summary: 处理搜索完成事件
 * Parameters: 无
 * Return: void
 * Description: 处理搜索完成事件，保存搜索记录到数据库并发出信号
 */
void SearchBridge::handleSearchComplete() {
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
}

/*
 * Summary: 获取搜索历史记录
 * Parameters: 无
 * Return: QVariantList - 包含搜索历史记录的列表
 * Description: 从数据库获取最近10条搜索历史记录
 */
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
            item["id"] = record.first;  // 第一个字段现在是记录ID
            item["query"] = record.second;  // 第二个字段是查询内容
            historyList.append(item);
        }
    }
    
    INFOLOG("Retrieved {} search history records", historyList.size());
    return historyList;
}


/*
 * Summary: 删除搜索历史记录
 * Parameters:
 *   const QString& query - 要删除的搜索记录内容
 * Return: void
 * Description: 根据搜索内容从数据库删除对应的历史记录
 */
void SearchBridge::deleteSearchHistory(const QString& recordId) {
    INFOLOG("Starting to delete search history record with ID: {}", recordId.toStdString());
    if(dbManager->deleteSearchHistory(recordId)) {
        emit searchHistoryChanged();
        INFOLOG("Successfully deleted search history record with ID: {}", recordId.toStdString());
    } else {
        WARNLOG("Failed to delete search history record with ID: {}", recordId.toStdString());
    }
}

} // namespace IntelliSearch