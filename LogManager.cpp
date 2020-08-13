#include "LogManager.h"

#include <QThread>
#include <QTextStream>

//重定向qdebug输出
void outputLogMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    LogManager::getInstance()->outputLog(type,context,msg);
}

LogManager::LogManager()
{
    initManager();
}

LogManager::~LogManager()
{
    freeManager();
}

LogManager *LogManager::getInstance()
{
    //单例，c++ 11
    static LogManager instance;
    return &instance;
}

void LogManager::outputLog(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString out_text;
    QTextStream stream(&out_text);
    stream<<msg<<QThread::currentThreadId();

    //如果要写文件需要加锁，因为函数调用在debug调用线程
    //file.write(msg.toUtf8()+"\n");
    emit newLog(msg);
    //默认的输出，控制台
    defaultOutput(type,context,out_text);
}

void LogManager::initManager()
{
    file.setFileName("log.txt");
    defaultOutput=qInstallMessageHandler(outputLogMessage);
}

void LogManager::freeManager()
{
    file.close();
    qInstallMessageHandler(nullptr);
}

