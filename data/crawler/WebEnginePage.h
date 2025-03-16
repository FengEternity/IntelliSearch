#ifndef INTELLISEARCH_WEBENGINEPAGE_H
#define INTELLISEARCH_WEBENGINEPAGE_H

#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineCertificateError>
#include <QString>
#include <QUrl>

namespace IntelliSearch
{
    // 自定义WebEnginePage类，用于处理动态网页
    class WebEnginePage : public QWebEnginePage
    {
        Q_OBJECT

    public:
        explicit WebEnginePage(QObject *parent = nullptr);
        ~WebEnginePage();

        // 设置用户代理
        void setUserAgent(const QString &userAgent);

    protected:
        // 重写certificateError以处理SSL错误
        bool certificateError(const QWebEngineCertificateError &error);

        // 重写javaScriptConsoleMessage以记录JavaScript控制台消息
        void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, 
                                     const QString &message, 
                                     int lineNumber, 
                                     const QString &sourceID);

    private:
        QWebEngineProfile *m_profile;
    };

} // namespace IntelliSearch

#endif // INTELLISEARCH_WEBENGINEPAGE_H