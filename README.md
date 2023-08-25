# SimpleQtLogger

Simple log class implemented by redirecting QDebug. 
通过重定向 QDebug 实现的简易日志类。

# Environment

Windows 10/11 + MSVC2019 + Qt5.15.2

# Note

- By default, context(QMessageLogContext) information is recorded only in debug builds. You can overwrite this explicitly by defining QT_MESSAGELOGCONTEXT or QT_NO_MESSAGELOGCONTEXT.
默认只有 Debug 模式才记录了上下文信息（QMessageLogContext）， 可通过宏定义 QT_MESSAGELOGCONTEXT 或 QT_NO_MESSAGELOGCONTEXT 打开或关闭。

- The function is very simple, so there is no interface document, but examples for QWidget and QML are provided.
功能非常简单，故没有接口文档，但是提供了 QWidget 和 QML 的示例。
