#include "SearchBridge.h"
#include "../../log/Logger.h"
#include <QDebug>

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
    INFOLOG("SearchBridge initialization completed");
}

SearchBridge::~SearchBridge() = default;

void SearchBridge::handleSearch(const QString& query) {
    INFOLOG("Received search request: {}", query.toStdString());
    DEBUGLOG("Search request encoding check - QString raw content: {}", query.toUtf8().toHex().toStdString());
    try {
        // Convert QString to std::string and call IntentParser
        std::string stdQuery = query.toStdString();
        DEBUGLOG("Search request encoding conversion - std::string conversion result: {}", stdQuery);
        
        auto result = intentParser->parseSearchIntent(stdQuery);
        
        // 将结果转换为 JSON 字符串并发送给 QML
        QString jsonString = QString::fromStdString(result.dump());
        
        // 保存搜索记录到数据库
        if (!dbManager->addSearchHistory(query, jsonString)) {
            WARNLOG("Failed to save search history");
        }
        
        DEBUGLOG("Search result encoding check - JSON conversion result: {}", jsonString.toUtf8().toHex().toStdString());
        INFOLOG("Search results ready, sending to frontend");
        emit searchResultsReady(jsonString);
    } catch (const std::exception& e) {
        ERRORLOG("Search failed: {}", e.what());
        qWarning() << "Search failed:" << e.what();
        emit searchResultsReady(QString("{\"error\":\"%1\"}").arg(e.what()));
    }
}

} // namespace IntelliSearch