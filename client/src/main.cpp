#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>

#include "log/Logger.h"
#include "config/ConfigManager.h"
#include "SearchBridge.h"

int main(int argc, char *argv[]) {
    // 初始化配置管理器
    ConfigManager::getInstance()->init("config/config.json");

    // 初始化日志
    INITLOG(ConfigManager::getInstance()->getLogConfig());
    INFOLOG("Application started");

    QGuiApplication app(argc, argv);
    
    // 设置Material风格
    QQuickStyle::setStyle("Material");
    DEBUGLOG("UI style set to Material");
    
    QQmlApplicationEngine engine;
    
    // 创建 SearchBridge 实例并设置为上下文属性
    IntelliSearch::SearchBridge *searchBridge = new IntelliSearch::SearchBridge();
    engine.rootContext()->setContextProperty("searchBridge", searchBridge);
    
    // 加载主QML文件
    const QUrl url("qrc:/qml/main.qml");
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
    
    DEBUGLOG("Starting to load QML file: {}", url.toString().toStdString());
    engine.load(url);

    return app.exec();
}