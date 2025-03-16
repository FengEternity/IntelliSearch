#ifndef INTELLISEARCH_JSEXECUTIONANALYZER_H
#define INTELLISEARCH_JSEXECUTIONANALYZER_H

#include <QString>
#include <QObject>
#include <QWebEnginePage>
#include <QWebEngineScript>
#include <QEventLoop>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

namespace IntelliSearch
{
    // JavaScript执行分析结果结构体
    struct JSExecutionResult
    {   
        int domMutationCount;       // DOM变化次数
        int ajaxRequestCount;        // AJAX请求次数
        int eventListenerCount;      // 事件监听器数量
        int timerCount;              // 定时器数量
        int storageAccessCount;      // 存储访问次数
        QJsonArray networkRequests;  // 网络请求详情
        QJsonArray domChanges;       // DOM变化详情
        bool hasDynamicContent;      // 是否包含动态内容
    };

    // JavaScript执行分析器类
    class JSExecutionAnalyzer : public QObject
    {   
        Q_OBJECT
        
    public:
        explicit JSExecutionAnalyzer(QObject *parent = nullptr);
        ~JSExecutionAnalyzer();
        
        // 分析页面JavaScript执行情况
        JSExecutionResult analyzeExecution(const QString &url, int timeout = 5000);
        
        // 获取页面快照（DOM结构）
        QString captureSnapshot(QWebEnginePage *page);
        
        // 比较两个DOM快照的差异
        double compareSnapshots(const QString &before, const QString &after);
        
    private:
        // 注入监控脚本
        void injectMonitoringScripts(QWebEnginePage *page);
        
        // 收集执行结果
        JSExecutionResult collectResults(QWebEnginePage *page);
        
        // 等待JavaScript执行
        void waitForExecution(QWebEnginePage *page, int timeout);
        
        // 监控脚本内容
        QString m_monitoringScript;
    };

} // namespace IntelliSearch

#endif // INTELLISEARCH_JSEXECUTIONANALYZER_H