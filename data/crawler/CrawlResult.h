#ifndef INTELLISEARCH_CRAWLRESULT_H
#define INTELLISEARCH_CRAWLRESULT_H

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QDateTime>

namespace IntelliSearch
{
    // 爬取结果结构体
    struct CrawlResult
    {
        QString url;          // 页面URL
        QString title;        // 页面标题
        QString content;      // 页面内容
        QStringList links;    // 页面中的链接
        QJsonObject metadata; // 元数据
        QDateTime timestamp;  // 爬取时间戳
    };

} // namespace IntelliSearch

#endif // INTELLISEARCH_CRAWLRESULT_H