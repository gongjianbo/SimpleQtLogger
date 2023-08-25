#pragma once
#include <QObject>
#include <QFile>
#include <QMutex>
#include <QDebug>

/**
 * @brief 简易的日志管理类，作为单例
 * @author 龚建波 - https://github.com/gongjianbo
 * @date 2020-08-13
 * @details
 * 1.初始化时调用 initManager 重定向 QDebug 输出
 * 析构时自动调用 freeManager，也可以手动调用 freeManager
 * 2.根据时间戳每天重新生成一个文件，超过文件大小也会重新生成
 */
class LogManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(LogManager)
    LogManager();
public:
    ~LogManager();

    // 获取单例实例
    static LogManager *getInstance();
    // 获取带 html 样式标签的富文本
    Q_INVOKABLE static QString richText(int msgType, const QString &log);

    // 初始化，如重定向等
    void initManager(const QString &dir = QString());
    // 释放
    void freeManager();

    // 文件最大大小，超过则新建文件，单位字节
    qint64 getFileSizeLimit() const;
    void setFileSizeLimit(qint64 limit);

private:
    // 重定向到此接口
    static void outputHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    // 获取重定向的打印信息，在静态函数种回调该接口
    void outputLog(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    // 计算下一次生成文件的时间
    qint64 calcNextTime() const;
    // 每次写入时判断是否打开，是否需要新建文件
    void prepareFile();

signals:
    // 可以关联信号接收日志信息，如显示到 ui 中
    // 注意，如果槽函数为 lambda 或者其他没有接收者的情况，需要保证槽函数中的变量有效性
    // 因为 static 变量的生命周期更长，可能槽函数所在模块已经释放资源，最好 connect 加上接收者
    void newLog(int msgType, const QString &log);

private:
    // 保留默认 handle，用于输出到控制台
    QtMessageHandler defaultOutput = nullptr;

    // 输出到文件
    QFile logFile;
    // 输出路径
    QString logDir;
    // 多线程操作时需要加锁
    mutable QMutex logMutex;

    // 下一次生成文件的时间戳，单位毫秒
    qint64 fileNextTime{ 0 };
    // 文件最大大小，超过则新建文件，单位字节
    qint64 fileSizeLimit{ 1024 * 1024 * 32 };
};
