#include "LogManager.h"
#include <QApplication>
#include <QDir>
#include <QThread>
#include <QTextStream>
#include <QDateTime>

LogManager::LogManager()
{
    //initManager();
}

LogManager::~LogManager()
{
    freeManager();
}

LogManager *LogManager::getInstance()
{
    // 单例，初次调用时实例化
    static LogManager instance;
    return &instance;
}

QString LogManager::richText(int msgType, const QString &log)
{
    QString log_text;
    QTextStream stream(&log_text);
    switch (msgType) {
    case QtDebugMsg: stream << "<span style='color:green;'>"; break;
    case QtInfoMsg: stream << "<span style='color:blue;'>"; break;
    case QtWarningMsg: stream << "<span style='color:gold;'>"; break;
    case QtCriticalMsg: stream << "<span style='color:red;'>"; break;
    case QtFatalMsg: stream << "<span style='color:red;'>"; break;
    default: stream << "<span style='color:red;'>"; break;
    }
    stream << log << "</span>";
    return log_text;
}

void LogManager::initManager(const QString &path)
{
    // 保存路径
    filePath = path;
    if (filePath.isEmpty())
    {
        // 用到了 QApplication::applicationDirPath()，需要先实例化一个app
        int argc = 0;
        QApplication app(argc,nullptr);
        filePath = app.applicationDirPath() + "/log";
    }
    QDir dir(filePath);
    if (!dir.exists())
    {
        // QFile 不会自动创建不存在的目录
        dir.mkpath(filePath);
    }
    elapsedTimer.start();
    // 重定向qdebug到自定义函数
    defaultOutput = qInstallMessageHandler(LogManager::outputHandler);
}

void LogManager::freeManager()
{
    file.close();
    qInstallMessageHandler(nullptr);
}

void LogManager::outputHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // 转发给单例的成员函数
    LogManager::getInstance()->outputLog(type, context, msg);
}

void LogManager::outputLog(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // widget 中的 log，context.category = default
    // qml 中的 log，context.category = qml，此时默认的 output 会增加一个 "qml:" 前缀输出
    //fprintf(stderr, "print: type = %d, category = %s \n", type, context.category);

    // 如果要写文件需要加锁，因为函数调用在 debug 调用线程
    QMutexLocker locker(&logMutex);

    QString out_text;
    QTextStream stream(&out_text);

    // 时间
    stream << QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss]");
    // 日志类型
    switch (type) {
    case QtDebugMsg: stream << "[Debug]"; break;
    case QtInfoMsg: stream << "[Info]"; break;
    case QtWarningMsg: stream << "[Warning]"; break;
    case QtCriticalMsg: stream << "[Critical]"; break;
    case QtFatalMsg: stream << "[Fatal]"; break;
    default: stream << "[Unknown]"; break;
    }
    // 线程 id
    stream << "[" << QThread::currentThreadId() << "]";
    // 输出位置
    stream << "[" << context.file << ":" << context.line << "]";
    // 日志信息
    stream << msg;

    //写入文件
    if (file.isOpen()) {
        // elapsed 距离 start 的毫秒数
        // 这里设置 1 分钟用来测试
        if (elapsedTimer.elapsed() > 1000 * 60){
            file.close();
            // 重新计时
            elapsedTimer.restart();
        }
    }
    if (!file.isOpen()) {
        // 新的文件
        file.setFileName(filePath + QString("/log_%1.txt")
                         .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmm")));
        // Append 追加模式，避免同一文件被清除
        if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
            emit newLog(QtWarningMsg, "Open log file error:" + file.errorString() + file.fileName());
        }
    }
    if (file.isOpen()) {
        // 写入文件
        stream.setDevice(&file);
        stream << out_text << Qt::endl;
    }

    // 发送信号给需要的对象，如 ui 上显示日志
    emit newLog(type, out_text);

    // 默认的输出，控制台
    // 区分日志类型给文本加颜色
    // 常见格式为：\e[显示方式;背景颜色;前景文字颜色m << 输出字符串 << \e[0m
    // 其中 \e=\033
    // -----------------
    // 背景色  字体色
    // 40:    30:    黑
    // 41:    31:    红
    // 42:    32:    绿
    // 43:    33:    黄
    // 44:    34:    蓝
    // 45:    35:    紫
    // 46:    36:    深绿
    // 47:    37:    白
    // -----------------
    QString cmd_text;
    stream.setString(&cmd_text);
    switch (type) {
    case QtDebugMsg: // debug 绿色
        stream << "\033[32m"; break;
    case QtInfoMsg: // info 蓝色
        stream << "\033[34m"; break;
    case QtWarningMsg: // warning 黄色
        stream << "\033[33m"; break;
    case QtCriticalMsg: // critical 红字
        stream << "\033[31m"; break;
    case QtFatalMsg: // fatal 黑底红字
        // qFatal 表示致命错误，默认处理会报异常的
        stream<<"\033[0;31;40m"; break;
    default: // defualt 默认颜色
        stream << "\033[0m"; break;
    }
    stream << out_text << "\033[0m";
    defaultOutput(type, context, cmd_text);
}
