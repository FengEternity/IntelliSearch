#include "HtmlParser.h"
#include "../../log/Logger.h"

namespace IntelliSearch
{

    HtmlParser::HtmlParser()
    {
        // 构造函数实现
    }

    HtmlParser::~HtmlParser()
    {
        // 析构函数实现
    }

    CrawlResult HtmlParser::parseHtml(const QString &url, const QString &html)
    {
        CrawlResult result;
        result.url = url;
        result.timestamp = QDateTime::currentDateTime();

        // 提取标题
        QRegularExpression titleRegex("<title>([^<]*)</title>", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch titleMatch = titleRegex.match(html);
        if (titleMatch.hasMatch())
        {
            result.title = titleMatch.captured(1).trimmed();
        }

        // 提取内容 (简单实现，移除HTML标签)
        QTextDocument doc;
        doc.setHtml(html);
        result.content = doc.toPlainText();

        // 提取链接
        result.links = extractLinks(url, html);

        INFOLOG("Parsed HTML for URL: {}", url.toStdString());
        INFOLOG("Parser result - Title: {}, Content length: {}, Links count: {}", 
            result.title.toStdString(), 
            result.content.length(), 
            result.links.size()
        );

        return result;
    }

    QStringList HtmlParser::extractLinks(const QString &baseUrl, const QString &html)
    {
        QStringList links;
        QSet<QString> uniqueLinks;

        // 提取href属性
        QRegularExpression hrefRegex("href=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator i = hrefRegex.globalMatch(html);

        while (i.hasNext())
        {
            QRegularExpressionMatch match = i.next();
            QString href = match.captured(1).trimmed();

            // 规范化URL
            QString normalizedUrl = normalizeUrl(baseUrl, href);

            // 检查URL是否有效且未被处理过
            if (!normalizedUrl.isEmpty() && !uniqueLinks.contains(normalizedUrl))
            {
                uniqueLinks.insert(normalizedUrl);
                links.append(normalizedUrl);
            }
        }

        return links;
    }

    QString HtmlParser::normalizeUrl(const QString &baseUrl, const QString &url, 
                                    bool followExternalLinks,
                                    const QStringList &allowedDomains,
                                    const QStringList &urlFilters)
    {
        // 创建QUrl对象
        QUrl qurl;
        if (url.startsWith("http://") || url.startsWith("https://"))
        {
            qurl = QUrl(url);
        }
        else
        {
            QUrl base(baseUrl);
            qurl = base.resolved(QUrl(url));
        }

        // 检查URL是否有效
        if (!qurl.isValid())
        {
            return QString();
        }

        // 移除片段(#后面的内容)
        qurl.setFragment(QString());

        // 获取规范化的URL字符串
        QString normalizedUrl = qurl.toString();

        // 检查是否应该跟随外部链接
        if (!followExternalLinks && !baseUrl.isEmpty())
        {
            QUrl baseQUrl(baseUrl);
            if (baseQUrl.host() != qurl.host())
            {
                return QString();
            }
        }

        // 检查是否在允许的域名列表中
        if (!allowedDomains.isEmpty())
        {
            bool allowed = false;
            for (const QString &domain : allowedDomains)
            {
                if (qurl.host().endsWith(domain, Qt::CaseInsensitive))
                {
                    allowed = true;
                    break;
                }
            }
            if (!allowed)
            {
                return QString();
            }
        }

        // 应用URL过滤规则
        if (!urlFilters.isEmpty())
        {
            for (const QString &filter : urlFilters)
            {
                QRegularExpression regex(filter);
                if (regex.match(normalizedUrl).hasMatch())
                {
                    return QString();
                }
            }
        }

        return normalizedUrl;
    }

} // namespace IntelliSearch