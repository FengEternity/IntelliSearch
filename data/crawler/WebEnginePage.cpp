#include "WebEnginePage.h"
#include "../../log/Logger.h"

namespace IntelliSearch
{

    WebEnginePage::WebEnginePage(QObject *parent)
        : QWebEnginePage(new QWebEngineProfile(), parent), m_profile(profile())
    {
        // 设置默认设置
        m_profile->setHttpUserAgent("Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) "
                                   "AppleWebKit/537.36 (KHTML, like Gecko) "
                                   "Chrome/91.0.4472.124 Safari/537.36");
        
        // 禁用持久化Cookie存储
        m_profile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
        
        // 禁用缓存
        m_profile->setHttpCacheType(QWebEngineProfile::MemoryHttpCache);
        
        // 禁用Web存储
        settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, false);
        
        // 启用JavaScript
        settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
        
        // 禁用插件
        settings()->setAttribute(QWebEngineSettings::PluginsEnabled, false);
        
        INFOLOG("WebEnginePage initialized");
    }

    WebEnginePage::~WebEnginePage()
    {
        // 析构函数实现
        m_profile->deleteLater();
    }

    void WebEnginePage::setUserAgent(const QString &userAgent)
    {
        m_profile->setHttpUserAgent(userAgent);
    }

    bool WebEnginePage::certificateError(const QWebEngineCertificateError &error)
    {
        // 记录证书错误但继续加载
        WARNLOG("Certificate error: {}", error.description().toStdString());
        return true; // 忽略错误并继续
    }

    void WebEnginePage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, 
                                               const QString &message, 
                                               int lineNumber, 
                                               const QString &sourceID)
    {
        // 记录JavaScript控制台消息
        switch (level) {
            case InfoMessageLevel:
                DEBUGLOG("JS Info [{}:{}]: {}", sourceID.toStdString(), lineNumber, message.toStdString());
                break;
            case WarningMessageLevel:
                WARNLOG("JS Warning [{}:{}]: {}", sourceID.toStdString(), lineNumber, message.toStdString());
                break;
            case ErrorMessageLevel:
                ERRORLOG("JS Error [{}:{}]: {}", sourceID.toStdString(), lineNumber, message.toStdString());
                break;
        }
    }

} // namespace IntelliSearch