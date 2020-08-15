#include "LogManager.h"

#include <QApplication>
#include <QDir>
#include <QThread>
#include <QTextStream>
#include <QDateTime>

//重定向qdebug输出
void outputLogMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    //转发给单例的成员函数
    LogManager::getInstance()->outputLog(type,context,msg);
}

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
    //单例，c++ 11
    static LogManager instance;
    return &instance;
}

void LogManager::outputLog(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    //如果要写文件需要加锁，因为函数调用在debug调用线程
    QMutexLocker locker(&logMutex);
    QString out_text;
    QTextStream stream(&out_text);
    //区分日志类型给文本加头
    switch (type) {
    case QtDebugMsg: stream<<"[Debug]"; break;
    case QtInfoMsg: stream<<"[Info]"; break;
    case QtWarningMsg: stream<<"[Warning]"; break;
    case QtCriticalMsg: stream<<"[Critical]"; break;
    case QtFatalMsg: stream<<"[Fatal]"; break;
    default: stream<<"[Unknown]"; break;
    }

    //加个线程id用于测试
    stream<<QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss]")
            <<msg
         <<QThread::currentThreadId();

    //写入文件
    if(file.isOpen()){
        //elapsed距离start的毫秒数
        //这里设置1分钟用来测试
        if(elapsedTimer.elapsed()>1000*60){
            file.close();
            //重新计时
            elapsedTimer.restart();
        }
    }
    if(!file.isOpen()){
        //新的文件
        file.setFileName(filePath+QString("/log_%1.txt")
                         .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmm")));
        //Append追加模式，避免同一文件被清除
        if(!file.open(QIODevice::WriteOnly|QIODevice::Append)){
            emit newLog(QtWarningMsg,"Open log file error:"+file.errorString()+file.fileName());
        }
    }
    if(file.isOpen()){
        //写入文件
        stream.setDevice(&file);
        stream<<out_text<<endl;
    }

    //发送信号给需要的对象，如ui上显示日志
    emit newLog(type, msg);

    //默认的输出，控制台
    //区分日志类型给文本加颜色
    //常见格式为：\e[显示方式;前景色;背景色m输出字符串\e[0m
    //其中\e=\033
    QString cmd_text;
    stream.setString(&cmd_text);
    switch (type) {
    //debug绿色
    case QtDebugMsg: stream<<"\033[32m"; break;
        //info蓝色
    case QtInfoMsg: stream<<"\033[34m"; break;
        //warning黄色
    case QtWarningMsg: stream<<"\033[33m"; break;
        //critical黑底白字
    case QtCriticalMsg: stream<<"\033[0;37;40m"; break;
        //fatal黑底红字
        //qFatal表示致命错误，默认处理会报异常的
    case QtFatalMsg: stream<<"\033[0;31;40m"; break;
        //defualt默认颜色
    default: stream<<"\033[0m"; break;
    }
    stream<<out_text<<"\033[0m";
    defaultOutput(type,context,cmd_text);
}

void LogManager::initManager(const QString &path)
{
    //保存路径
    filePath=path;
    if(filePath.isEmpty())
    {
        //用到了QApplication::applicationDirPath()，需要先实例化一个app
        int argc=0;
        QApplication app(argc,nullptr);
        filePath=app.applicationDirPath()+"/log";
    }
    QDir dir(filePath);
    if(!dir.exists())
    {
        //虽然QFile能够创建不存在的文件，但是它就是不会自动创建不存在的目录
        dir.mkpath(filePath);
    }
    elapsedTimer.start();
    //重定向qdebug到自定义函数
    defaultOutput=qInstallMessageHandler(outputLogMessage);
}

void LogManager::freeManager()
{
    file.close();
    qInstallMessageHandler(nullptr);
}

