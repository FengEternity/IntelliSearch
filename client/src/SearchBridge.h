#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <memory>
#include "core/engine/IntentParser.h"
#include "../../data/database/DatabaseManager.h"
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

namespace IntelliSearch {

class SearchBridge : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList searchHistory READ getSearchHistory NOTIFY searchHistoryChanged)
    Q_PROPERTY(bool isSearching READ isSearching NOTIFY searchingChanged)

public:
    explicit SearchBridge(QObject* parent = nullptr);
    ~SearchBridge();

public slots:
    void handleSearch(const QString& query);
    void deleteSearchHistory(const QString& query);
    QVariantList getSearchHistory();

signals:
    void searchResultsReady(const QString& results);
    void searchHistoryChanged();
    void searchingChanged();

private slots:
    void handleSearchComplete();

private:
    std::unique_ptr<IntentParser> intentParser;
    std::unique_ptr<IDatabaseManager> dbManager;
    QFutureWatcher<QString> searchWatcher;
    QString lastQuery;

    bool isSearching() const { return searchWatcher.isRunning(); }
};

} // namespace IntelliSearch