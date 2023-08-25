#include "LogManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QThread>
#include <QTextStream>
#include <QDateTime>

LogManager::LogManager()
{

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

void LogManager::initManager(const QString &dir)
{
    QMutexLocker locker(&logMutex);

    // 保存路径
    logDir = dir;
    if (logDir.isEmpty())
    {
        // 用到了 QCoreApplication::applicationDirPath()，需要先实例化一个app
        if (qApp) {
            logDir = qApp->applicationDirPath() + "/log";
        } else {
            int argc = 0;
            QCoreApplication app(argc,nullptr);
            logDir = app.applicationDirPath() + "/log";
        }
    }

    // 计算下次创建文件的时间点
    fileNextTime = calcNextTime();
    // 重定向qdebug到自定义函数
    defaultOutput = qInstallMessageHandler(LogManager::outputHandler);
}

void LogManager::freeManager()
{
    QMutexLocker locker(&logMutex);

    logFile.close();
    if (defaultOutput) {
        qInstallMessageHandler(defaultOutput);
        defaultOutput = nullptr;
    }
}

qint64 LogManager::getFileSizeLimit() const
{
    return fileSizeLimit;
}

void LogManager::setFileSizeLimit(qint64 limit)
{
    fileSizeLimit = limit;
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
    // fprintf(stderr, "print: type = %d, category = %s \n", type, context.category);

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

    // 判断是否需要打开或者新建文件
    prepareFile();
    if (logFile.isOpen()) {
        // 写入文件
        stream.setDevice(&logFile);
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
        stream << "\033[0;31;40m"; break;
    default: // defualt 默认颜色
        stream << "\033[0m"; break;
    }
    stream << out_text << "\033[0m";
    defaultOutput(type, context, cmd_text);
}

qint64 LogManager::calcNextTime() const
{
    // 可以参考 spdlog 的 daily_file_sink 优化，这里先用 Qt 接口进行实现
    return QDate::currentDate().addDays(1).startOfDay().toMSecsSinceEpoch();
}

void LogManager::prepareFile()
{
    // 写入文件
    // 先计算好下一次生成文件的时间点，然后和当前进行比较，这里没有考虑调节系统日期的情况
    if (fileNextTime <= QDateTime::currentDateTime().toMSecsSinceEpoch()){
        logFile.close();
        // 计算下次创建文件的时间点
        fileNextTime = calcNextTime();
    }
    // 文件超过了大小
    if (logFile.isOpen() && logFile.size() >= fileSizeLimit) {
        logFile.close();
    }
    // 生成文件名，打开文件
    if (!logFile.isOpen()) {
        // 创建文件前创建目录，QFile 不会自动创建不存在的目录
        QDir dir(logDir);
        if (!dir.exists()) {
            dir.mkpath(logDir);
        }
        // 文件日期
        QString file_day = QDate::currentDate().toString("yyyyMMdd");
        QString file_path = QString("%1/log_%2.txt").arg(logDir).arg(file_day);
        logFile.setFileName(file_path);
        if (logFile.exists() && logFile.size() >= fileSizeLimit) {
            QString file_time = QTime::currentTime().toString("hhmmss");
            file_path = QString("%1/log_%2_%3.txt").arg(logDir).arg(file_day).arg(file_time);
            logFile.setFileName(file_path);
        }
        // 打开新的文件
        // Append 追加模式，避免同一文件被清除
        if (!logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            emit newLog(QtWarningMsg, "Open log file error:" + logFile.errorString() + logFile.fileName());
        }
    }
}
