#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    
    // 设置Material风格
    QQuickStyle::setStyle("Material");
    
    QQmlApplicationEngine engine;
    
    // 加载主QML文件
    const QUrl url("qrc:/qml/main.qml");
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection
    );
    
    engine.load(url);
    
    return app.exec();
}