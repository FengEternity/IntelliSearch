#pragma once

#include <QObject>
#include <QString>
#include <memory>
#include "core/engine/IntentParser.h"
#include "../../data/database/DatabaseManager.h"

namespace IntelliSearch {

class SearchBridge : public QObject {
    Q_OBJECT

public:
    explicit SearchBridge(QObject* parent = nullptr);
    ~SearchBridge();

public slots:
    void handleSearch(const QString& query);

signals:
    void searchResultsReady(const QString& results);

private:
    std::unique_ptr<IntentParser> intentParser;
    std::unique_ptr<IDatabaseManager> dbManager;
};

} // namespace IntelliSearch