#ifndef INTELLISEARCH_SEARCHBRIDGE_H
#define INTELLISEARCH_SEARCHBRIDGE_H

#include <QObject>
#include <QString>
#include "../../core/src/IntentParser.h"

namespace IntelliSearch {

class SearchBridge : public QObject {
    Q_OBJECT

public:
    explicit SearchBridge(QObject* parent = nullptr);
    ~SearchBridge();

public slots:
    // 处理来自 QML 的搜索请求
    void handleSearch(const QString& query);

signals:
    // 向 QML 发送搜索结果
    void searchResultsReady(const QString& results);

private:
    std::unique_ptr<IntentParser> intentParser;
};

} // namespace IntelliSearch

#endif // INTELLISEARCH_SEARCHBRIDGE_H