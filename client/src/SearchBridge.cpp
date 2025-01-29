#include "SearchBridge.h"
#include <QDebug>

namespace IntelliSearch {

SearchBridge::SearchBridge(QObject* parent)
    : QObject(parent)
    , intentParser(std::make_unique<IntentParser>())
{
}

SearchBridge::~SearchBridge() = default;

void SearchBridge::handleSearch(const QString& query) {
    try {
        // 将 QString 转换为 std::string 并调用 IntentParser
        auto result = intentParser->parseSearchIntent(query.toStdString());
        
        // 将结果转换为 JSON 字符串并发送给 QML
        QString jsonString = QString::fromStdString(result.dump());
        emit searchResultsReady(jsonString);
    } catch (const std::exception& e) {
        qWarning() << "Search failed:" << e.what();
        emit searchResultsReady(QString("{\"error\":\"%1\"}").arg(e.what()));
    }
}

} // namespace IntelliSearch