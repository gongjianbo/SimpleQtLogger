#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QObject>
#include <QFile>
#include <QDebug>

/**
 * @brief 简易的日志管理类，作为单例
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

signals:
    //可以关联信号接收日志信息，如显示到ui中
    void newLog(const QString &log);

private:
    //初始化，如重定向等
    void initManager();
    //释放
    void freeManager();

private:
    //保留默认handle，用于输出到控制台
    QtMessageHandler defaultOutput = nullptr;
    //输出到文件
    QFile file;
};

#endif // LOGMANAGER_H
