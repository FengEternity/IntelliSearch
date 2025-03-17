#include "PythonCrawlerBridge.h"
#include "../../log/Logger.h"

namespace IntelliSearch
{

    PythonCrawlerBridge::PythonCrawlerBridge(QObject *parent)
        : QObject(parent), m_process(std::make_unique<QProcess>()), m_status(PythonCrawlerStatus::Idle),
          m_crawledCount(0), m_totalCount(0)
    {
        // 设置默认配置
        m_config.maxDepth = 1;
        m_config.maxPages = 1;
        m_config.requestDelay = 1000;
        m_config.followExternalLinks = false;
        m_config.useDynamicCrawling = false;
        m_config.pageLoadTimeout = 30000;
        
        // 设置Python相关路径
        m_config.pythonPath = "python3";
        m_config.crawlerScript = QDir::currentPath() + "/data/crawler/python_crawler/crawler.py";
        m_config.configPath = QDir::currentPath() + "/data/crawler/python_crawler/config.json";
        m_config.outputDir = QDir::currentPath() + "/data/crawler/python_crawler/results";
        
        // 确保输出目录存在
        QDir dir(m_config.outputDir);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        
        // 连接进程信号
        connect(m_process.get(), &QProcess::readyReadStandardOutput,
                this, &PythonCrawlerBridge::handleProcessOutput);
        connect(m_process.get(), &QProcess::readyReadStandardError,
                this, &PythonCrawlerBridge::handleProcessError);
        connect(m_process.get(), static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                this, &PythonCrawlerBridge::handleProcessFinished);
        
        // 设置结果检查定时器
        connect(&m_resultsCheckTimer, &QTimer::timeout,
                this, &PythonCrawlerBridge::checkResultsFile);
        
        INFOLOG("PythonCrawlerBridge initialized");
    }

    PythonCrawlerBridge::~PythonCrawlerBridge()
    {
        stopCrawling();
    }

    void PythonCrawlerBridge::startCrawling(const QString &url)
    {
        startCrawling(QStringList() << url);
    }

    void PythonCrawlerBridge::startCrawling(const QStringList &urls)
    {
        if (m_status == PythonCrawlerStatus::Running)
        {
            WARNLOG("Python crawler is already running");
            return;
        }

        // 清空之前的结果
        m_results.clear();
        m_crawledCount = 0;
        m_totalCount = 0;
        
        // 生成配置文件
        if (!generateConfigFile())
        {
            ERRORLOG("Failed to generate config file");
            emit errorOccurred("Failed to generate config file");
            return;
        }
        
        // 构建命令行参数
        QStringList args;
        args << m_config.crawlerScript;
        
        // 添加URL参数
        for (const QString &url : urls)
        {
            args << url;
        }
        
        // 添加配置文件参数
        args << "--config" << m_config.configPath;
        
        // 启动Python进程
        m_process->start(m_config.pythonPath, args);
        
        if (!m_process->waitForStarted(5000))
        {
            ERRORLOG("Failed to start Python crawler process: {}", m_process->errorString().toStdString());
            emit errorOccurred("Failed to start Python crawler process: " + m_process->errorString());
            return;
        }
        
        // 更新状态
        m_status = PythonCrawlerStatus::Running;
        emit statusChanged(m_status);
        
        // 启动结果检查定时器
        m_resultsCheckTimer.start(2000); // 每2秒检查一次结果文件
        
        INFOLOG("Started Python crawler with {} URLs", urls.size());
    }

    void PythonCrawlerBridge::pauseCrawling()
    {
        if (m_status == PythonCrawlerStatus::Running)
        {
            // 发送暂停命令到Python进程
            if (sendCommand("pause"))
            {
                m_status = PythonCrawlerStatus::Paused;
                emit statusChanged(m_status);
                INFOLOG("Paused Python crawler");
            }
        }
    }

    void PythonCrawlerBridge::resumeCrawling()
    {
        if (m_status == PythonCrawlerStatus::Paused)
        {
            // 发送恢复命令到Python进程
            if (sendCommand("resume"))
            {
                m_status = PythonCrawlerStatus::Running;
                emit statusChanged(m_status);
                INFOLOG("Resumed Python crawler");
            }
        }
    }

    void PythonCrawlerBridge::stopCrawling()
    {
        // 停止结果检查定时器
        m_resultsCheckTimer.stop();
        
        if (m_process && m_process->state() != QProcess::NotRunning)
        {
            // 发送停止命令到Python进程
            sendCommand("stop");
            
            // 等待进程退出
            if (!m_process->waitForFinished(5000))
            {
                WARNLOG("Python crawler process did not exit gracefully, terminating");
                m_process->terminate();
                
                if (!m_process->waitForFinished(3000))
                {
                    ERRORLOG("Failed to terminate Python crawler process, killing");
                    m_process->kill();
                }
            }
        }
        
        if (m_status != PythonCrawlerStatus::Idle && m_status != PythonCrawlerStatus::Completed)
        {
            m_status = PythonCrawlerStatus::Idle;
            emit statusChanged(m_status);
            INFOLOG("Stopped Python crawler");
        }
    }

    void PythonCrawlerBridge::setConfig(const PythonCrawlerConfig &config)
    {
        m_config = config;
    }

    PythonCrawlerConfig PythonCrawlerBridge::getConfig() const
    {
        return m_config;
    }

    PythonCrawlerStatus PythonCrawlerBridge::getStatus() const
    {
        return m_status;
    }

    int PythonCrawlerBridge::getCrawledCount() const
    {
        return m_crawledCount;
    }

    int PythonCrawlerBridge::getTotalCount() const
    {
        return m_totalCount;
    }

    void PythonCrawlerBridge::handleProcessOutput()
    {
        QByteArray output = m_process->readAllStandardOutput();
        QString outputStr = QString::fromUtf8(output).trimmed();
        
        if (!outputStr.isEmpty())
        {
            // 解析输出中的进度信息
            QRegularExpression progressRegex("Progress: (\\d+)/(\\d+)");
            QRegularExpressionMatch match = progressRegex.match(outputStr);
            
            if (match.hasMatch())
            {
                m_crawledCount = match.captured(1).toInt();
                m_totalCount = match.captured(2).toInt();
                
                emit progressChanged(m_crawledCount, m_totalCount);
            }
            
            // 检查是否有结果文件路径
            QRegularExpression resultFileRegex("Results saved to: (.+)");
            match = resultFileRegex.match(outputStr);
            
            if (match.hasMatch())
            {
                m_currentResultsFile = match.captured(1);
                INFOLOG("Python crawler results file: {}", m_currentResultsFile.toStdString());
                
                // 解析结果文件
                parseResults(m_currentResultsFile);
            }
            
            DEBUGLOG("Python crawler output: {}", outputStr.toStdString());
        }
    }

    void PythonCrawlerBridge::handleProcessError()
    {
        QByteArray error = m_process->readAllStandardError();
        QString errorStr = QString::fromUtf8(error).trimmed();
        
        if (!errorStr.isEmpty())
        {
            // 解析日志级别
            QRegularExpression logLevelRegex("- (INFO|WARN|DEBUG|ERROR) -");
            QRegularExpressionMatch match = logLevelRegex.match(errorStr);
            
            if (match.hasMatch()) {
                QString logLevel = match.captured(1);
                
                if (logLevel == "INFO") {
                    INFOLOG("Python crawler: {}", errorStr.toStdString());
                } else if (logLevel == "WARN") {
                    WARNLOG("Python crawler: {}", errorStr.toStdString());
                } else if (logLevel == "DEBUG") {
                    DEBUGLOG("Python crawler: {}", errorStr.toStdString());
                } else if (logLevel == "ERROR") {
                    ERRORLOG("Python crawler error: {}", errorStr.toStdString());
                    emit errorOccurred(errorStr);
                }
            } else {
                // 如果无法识别日志级别，默认当作错误处理
                ERRORLOG("Python crawler error: {}", errorStr.toStdString());
                emit errorOccurred(errorStr);
            }
        }
    }

    void PythonCrawlerBridge::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
    {
        m_resultsCheckTimer.stop();
        
        if (exitStatus == QProcess::NormalExit && exitCode == 0)
        {
            // 正常完成
            m_status = PythonCrawlerStatus::Completed;
            emit statusChanged(m_status);
            emit crawlingCompleted();
            INFOLOG("Python crawler completed successfully");
        }
        else
        {
            // 异常退出
            m_status = PythonCrawlerStatus::Error;
            emit statusChanged(m_status);
            QString errorMessage = QString("Python crawler process exited with code %1").arg(exitCode);
            emit errorOccurred(errorMessage);
            ERRORLOG("Python crawler process exited with code {}", exitCode);
        }
    }

    void PythonCrawlerBridge::checkResultsFile()
    {
        if (m_currentResultsFile.isEmpty())
        {
            return;
        }
        
        QFile file(m_currentResultsFile);
        if (file.exists())
        {
            // 解析结果文件
            parseResults(m_currentResultsFile);
        }
    }

    bool PythonCrawlerBridge::generateConfigFile()
    {
        QJsonObject config;
        
        // 转换配置参数为JSON格式
        config["max_depth"] = m_config.maxDepth;
        config["max_pages"] = m_config.maxPages;
        config["request_delay"] = m_config.requestDelay / 1000.0; // 转换为秒
        config["follow_external_links"] = m_config.followExternalLinks;
        config["use_dynamic_crawling"] = m_config.useDynamicCrawling;
        config["page_load_timeout"] = m_config.pageLoadTimeout / 1000; // 转换为秒
        
        // 转换字符串列表
        QJsonArray allowedDomains;
        for (const QString &domain : m_config.allowedDomains)
        {
            allowedDomains.append(domain);
        }
        config["allowed_domains"] = allowedDomains;
        
        QJsonArray urlFilters;
        for (const QString &filter : m_config.urlFilters)
        {
            urlFilters.append(filter);
        }
        config["url_filters"] = urlFilters;
        
        // 设置输出目录
        config["output_dir"] = m_config.outputDir;
        
        // 写入配置文件
        QFile file(m_config.configPath);
        if (!file.open(QIODevice::WriteOnly))
        {
            ERRORLOG("Failed to open config file for writing: {}", m_config.configPath.toStdString());
            return false;
        }
        
        QJsonDocument doc(config);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        
        INFOLOG("Generated Python crawler config file: {}", m_config.configPath.toStdString());
        return true;
    }

    void PythonCrawlerBridge::parseResults(const QString &resultsFile)
    {
        QFile file(resultsFile);
        if (!file.open(QIODevice::ReadOnly))
        {
            ERRORLOG("Failed to open results file: {}", resultsFile.toStdString());
            return;
        }
        
        QByteArray data = file.readAll();
        file.close();
        
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() || !doc.isArray())
        {
            ERRORLOG("Invalid JSON format in results file: {}", resultsFile.toStdString());
            return;
        }
        
        QJsonArray resultsArray = doc.array();
        
        // 处理新的结果
        for (int i = 0; i < resultsArray.size(); ++i)
        {
            QJsonObject resultObj = resultsArray[i].toObject();
            
            // 检查是否已经处理过该结果
            QString url = resultObj["url"].toString();
            bool alreadyProcessed = false;
            
            for (const CrawlResult &existingResult : m_results)
            {
                if (existingResult.url == url)
                {
                    alreadyProcessed = true;
                    break;
                }
            }
            
            if (!alreadyProcessed)
            {
                // 转换为CrawlResult并添加到结果列表
                CrawlResult result = convertToCrawlResult(resultObj);
                m_results.append(result);
                
                // 发送结果信号
                emit resultReady(result);
            }
        }
        
        // 更新爬取计数
        m_crawledCount = m_results.size();
        emit progressChanged(m_crawledCount, m_totalCount);
    }

    CrawlResult PythonCrawlerBridge::convertToCrawlResult(const QJsonObject &jsonResult)
    {
        CrawlResult result;
        
        // 设置基本属性
        result.url = jsonResult["url"].toString();
        result.title = jsonResult["title"].toString();
        result.content = jsonResult["content"].toString();
        
        // 设置链接列表
        QJsonArray linksArray = jsonResult["links"].toArray();
        for (int i = 0; i < linksArray.size(); ++i)
        {
            result.links.append(linksArray[i].toString());
        }
        
        // 设置元数据
        if (jsonResult.contains("metadata") && jsonResult["metadata"].isObject())
        {
            result.metadata = jsonResult["metadata"].toObject();
        }
        
        // 设置时间戳
        if (jsonResult.contains("timestamp") && jsonResult["timestamp"].isString())
        {
            QString timestampStr = jsonResult["timestamp"].toString();
            result.timestamp = QDateTime::fromString(timestampStr, Qt::ISODate);
        }
        else
        {
            result.timestamp = QDateTime::currentDateTime();
        }
        
        return result;
    }

    bool PythonCrawlerBridge::sendCommand(const QString &command)
    {
        if (m_process && m_process->state() == QProcess::Running)
        {
            // 向Python进程的标准输入写入命令
            QByteArray cmdData = (command + "\n").toUtf8();
            qint64 bytesWritten = m_process->write(cmdData);
            
            if (bytesWritten == cmdData.size())
            {
                INFOLOG("Sent command to Python crawler: {}", command.toStdString());
                return true;
            }
            else
            {
                ERRORLOG("Failed to send command to Python crawler: {}", command.toStdString());
                return false;
            }
        }
        
        WARNLOG("Cannot send command, Python crawler process is not running");
        return false;
    }
} // namespace IntelliSearch