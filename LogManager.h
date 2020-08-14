#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QObject>
#include <QFile>
#include <QMutex>
#include <QElapsedTimer>
#include <QDebug>

/**
 * @brief 简易的日志管理类，作为单例
 * @details
 * 初始化时调用initManager重定向
 * @note
 * 改为手动调用initManager是为了便于流程控制
 * 此外，也可以手动调用freeManager
 */
class LogManager : public QObject
{
    Q_OBJECT
    LogManager();
public:
    ~LogManager();

    //获取单例实例
    static LogManager *getInstance();
    //重定向qdebug输出
    void outputLog(QtMsgType type, const QMessageLogContext& context, const QString& msg);
    //初始化，如重定向等
    void initManager(const QString &path=QString());
    //释放
    void freeManager();

signals:
    //可以关联信号接收日志信息，如显示到ui中
    void newLog(const QString &log);

private:
    //保留默认handle，用于输出到控制台
    QtMessageHandler defaultOutput = nullptr;
    //输出到文件
    QFile file;
    //输出路径
    QString filePath;
    //多线程操作时需要加锁
    QMutex logMutex;
    //计算操作间隔，分时生成文件
    QElapsedTimer elapsedTimer;
};

#endif // LOGMANAGER_H
