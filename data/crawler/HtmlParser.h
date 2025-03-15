#ifndef INTELLISEARCH_HTMLPARSER_H
#define INTELLISEARCH_HTMLPARSER_H

#include <QString>
#include <QStringList>
#include <QUrl>
#include <QRegularExpression>
#include <QTextDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QSet>

namespace IntelliSearch
{
    // 爬取结果结构体（从Crawler.h中移植）
    struct CrawlResult
    {
        QString url;          // 页面URL
        QString title;        // 页面标题
        QString content;      // 页面内容
        QStringList links;    // 页面中的链接
        QJsonObject metadata; // 元数据
        QDateTime timestamp;  // 爬取时间戳
    };

    // HTML解析器类
    class HtmlParser
    {
    public:
        explicit HtmlParser();
        ~HtmlParser();

        // 解析HTML内容
        CrawlResult parseHtml(const QString &url, const QString &html);

        // 提取链接
        QStringList extractLinks(const QString &baseUrl, const QString &html);

        // 规范化URL
        QString normalizeUrl(const QString &baseUrl, const QString &url, 
                            bool followExternalLinks = false,
                            const QStringList &allowedDomains = QStringList(),
                            const QStringList &urlFilters = QStringList());

    private:
        // 可以添加私有成员变量和方法
    };

} // namespace IntelliSearch

#endif // INTELLISEARCH_HTMLPARSER_H