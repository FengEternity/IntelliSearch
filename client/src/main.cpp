#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>

#include "log/Logger.h"
#include "SearchBridge.h"

int main(int argc, char *argv[]) {
    // 配置日志
    LogConfig config;
    config.level = "debug";
    config.path = "logs/app.log";
    config.size = 1024 * 1024; // 1MB
    config.count = 3;

    // 初始化日志
    INITLOG(config);
    INFOLOG("程序启动");

    QGuiApplication app(argc, argv);
    
    // 设置Material风格
    QQuickStyle::setStyle("Material");
    DEBUGLOG("设置UI风格为Material");
    
    QQmlApplicationEngine engine;
    
    // 注册 SearchBridge 类型
    qmlRegisterType<IntelliSearch::SearchBridge>("IntelliSearch", 1, 0, "SearchBridge");
    
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
            ERRORLOG("QML对象创建失败");
            QCoreApplication::exit(-1); 
        },
        Qt::QueuedConnection
    );
    
    DEBUGLOG("开始加载QML文件: {}", url.toString().toStdString());
    engine.load(url);
    
    return app.exec();
}