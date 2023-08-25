#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QLoggingCategory>
#include "LogManager.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    // Qt 的 filter 可以过滤日志内容，这时重定向内也是接收不到的
    // QLoggingCategory::setFilterRules("*.warning=false");
    // 初始化
    LogManager::getInstance()->initManager();

    QApplication app(argc, argv);

    // Widget 窗口
    MainWindow window;
    window.show();

    // QML 窗口
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("logManager", LogManager::getInstance());
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
