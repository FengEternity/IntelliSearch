#include "HtmlParser.h"
#include "WebEnginePage.h"
#include "../../log/Logger.h"

namespace IntelliSearch
{

    HtmlParser::HtmlParser() : m_loadFinished(false)
    {
        // 构造函数实现
    }

    bool HtmlParser::needsDynamicCrawling(const QString &url, const QString &html) {
        // 强特征：直接判定
        if (html.contains("__NEXT_DATA__") || 
            html.contains("ReactDOM.render(") || 
            html.contains("fetch(\"/api/") || 
            html.contains("XMLHttpRequest")) {
            DEBUGLOG("Strong dynamic feature detected");
            return true;
        }
    
        // 弱特征组合：需满足至少两个条件
        int weakConditionsMet = 0;
    
        // 条件1：URL包含动态路径
        if (url.contains(QRegularExpression("/(spa|api|graphql)/", QRegularExpression::CaseInsensitiveOption))) {
            weakConditionsMet++;
        }
    
        // 条件2：存在模板语法
        if (html.contains(QRegularExpression("\\{\\{.*?\\}\\}"))) {
            weakConditionsMet++;
        }
    
        // 条件3：脚本数量 > 5 且包含动态关键词
        if (html.count("<script") > 5 && html.contains("data-binding")) {
            weakConditionsMet++;
        }
    
        // 至少满足两个弱特征才判定
        if (weakConditionsMet >= 2) {
            DEBUGLOG("Multiple weak conditions met");
            return true;
        }
    
        return false;
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
        // 打印解析结果
        INFOLOG("Parser Content: {}", result.content.toStdString());


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
    
    QWebEnginePage* HtmlParser::createWebPage()
    {
        // 创建自定义WebEnginePage实例
        WebEnginePage *page = new WebEnginePage();
        return page;
    }
    
    bool HtmlParser::waitForPageLoad(QWebEnginePage *page, int timeout)
    {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        
        // 连接页面加载完成信号
        QObject::connect(page, &QWebEnginePage::loadFinished, [&](bool success) {
            m_loadFinished = success;
            loop.quit();
        });
        
        // 连接超时信号
        QObject::connect(&timer, &QTimer::timeout, [&]() {
            WARNLOG("Page load timeout");
            loop.quit();
        });
        
        // 启动超时计时器
        timer.start(timeout);
        
        // 等待页面加载完成或超时
        loop.exec();
        
        return m_loadFinished;
    }
    
    CrawlResult HtmlParser::parseDynamicHtml(const QString &url, int timeout)
    {
        INFOLOG("Parsing dynamic HTML for URL: {}", url.toStdString());
        
        CrawlResult result;
        result.url = url;
        result.timestamp = QDateTime::currentDateTime();
        
        // 创建WebEnginePage实例
        QWebEnginePage *page = createWebPage();
        
        // 加载URL
        page->load(QUrl(url));
        
        // 等待页面加载完成
        if (!waitForPageLoad(page, timeout))
        {
            ERRORLOG("Failed to load page: {}", url.toStdString());
            delete page;
            return result;
        }
        
        // 获取页面标题
        result.title = page->title();
        
        // 获取页面HTML内容
        QEventLoop loop;
        page->toHtml([&](const QString &html) {
            // 提取内容
            QTextDocument doc;
            doc.setHtml(html);
            result.content = doc.toPlainText();
            
            // 提取链接
            result.links = extractLinks(url, html);
            
            loop.quit();
        });
        
        // 等待HTML内容获取完成
        loop.exec();
        
        INFOLOG("Parsed dynamic HTML for URL: {}", url.toStdString());
        INFOLOG("Parser result - Title: {}, Content length: {}, Links count: {}", 
            result.title.toStdString(), 
            result.content.length(), 
            result.links.size()
        );
        
        // 清理资源
        delete page;
        
        return result;
    }

} // namespace IntelliSearch