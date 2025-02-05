#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <memory>
#include "core/engine/IntentParser.h"
#include "../../data/database/DatabaseManager.h"

namespace IntelliSearch {

class SearchBridge : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList searchHistory READ getSearchHistory NOTIFY searchHistoryChanged)

public:
    explicit SearchBridge(QObject* parent = nullptr);
    ~SearchBridge();

public slots:
    void handleSearch(const QString& query);
    QVariantList getSearchHistory();

signals:
    void searchResultsReady(const QString& results);
    void searchHistoryChanged();

private:
    std::unique_ptr<IntentParser> intentParser;
    std::unique_ptr<IDatabaseManager> dbManager;
};

} // namespace IntelliSearch