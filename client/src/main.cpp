#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>
#include <QIcon>
#include <QFile>
#include <QGuiApplication>
#include <QJSEngine>

#include "log/Logger.h"
#include "config/ConfigManager.h"
#include "SearchBridge.h"
#include "../../data/crawler/CrawlerManager.h"

// QML日志处理函数
void qmlMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    switch (type) {
        case QtDebugMsg:
            QMLDEBUG(msg);
            break;
        case QtInfoMsg:
            QMLINFO(msg);
            break;
        case QtWarningMsg:
            QMLWARN(msg);
            break;
        case QtCriticalMsg:
        case QtFatalMsg:
            QMLERROR(msg);
            break;
    }
}

int main(int argc, char *argv[]) {
    // 初始化配置管理器
    ConfigManager::getInstance()->init("config/config.json");

    // 初始化日志
    INITLOG(ConfigManager::getInstance()->getLogConfig());
    INFOLOG("Application started");

    // 安装QML消息处理器
    qInstallMessageHandler(qmlMessageHandler);

    QApplication app(argc, argv);
    
    // 设置应用图标时最好先确认文件是否存在
    QString iconPath = ":/resources/icons/logo.svg";
    if (QFile::exists(iconPath)) {
        QApplication::setWindowIcon(QIcon(iconPath));
        DEBUGLOG("Window icon set successfully");
    } else {
        WARNLOG("Icon file not found: {}", iconPath.toStdString());
    }
    
    // 设置Material风格
    QQuickStyle::setStyle("Material");
    
    QQmlApplicationEngine engine;
    
    // 注册Logger实例到QML上下文
    engine.rootContext()->setContextProperty("logger", Logger::getInstance());
    
    // 使用qmlRegisterType注册SearchBridge类型
    qmlRegisterType<IntelliSearch::SearchBridge>("IntelliSearch", 1, 0, "SearchBridge");
    DEBUGLOG("SearchBridge type registered to QML");
    
    // 注册CrawlerManager类型到QML
    qmlRegisterType<IntelliSearch::CrawlerManager>("IntelliSearch", 1, 0, "CrawlerManager");
    DEBUGLOG("CrawlerManager type registered to QML");
    
    // 加载主QML文件
    const QUrl url("qrc:/main.qml");
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() {
            ERRORLOG("QML object creation failed");
            QCoreApplication::exit(-1); 
        },
        Qt::QueuedConnection
    );
    engine.load(url);

    return app.exec();
}