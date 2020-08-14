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
    QTextStream stream(&out_text); //加个线程id用于测试
    stream<<msg<<QThread::currentThreadId();

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
            emit newLog("Open log file error:"+file.errorString()+file.fileName());
        }
    }
    if(file.isOpen()){
        //写入文件
        stream.setDevice(&file);
        stream<<out_text<<endl;
    }

    //发送信号给需要的对象，如ui上显示日志
    emit newLog(msg);
    //默认的输出，控制台
    defaultOutput(type,context,out_text);
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

