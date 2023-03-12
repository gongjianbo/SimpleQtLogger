#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "LogManager.h"
#include <QDir>
#include <QDateTime>
#include <QThread>
#include <QtConcurrentRun>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 关联 log 信息，显示到文本框
    // 注意，如果槽函数为 lambda 或者其他没有接收者的情况，需要保证槽函数中的变量有效性
    // 因为 static 变量的生命周期更长，可能槽函数所在模块已经释放资源，最好 connect 加上接收者
    connect(LogManager::getInstance(), &LogManager::newLog,
            this, [this](int msgType, const QString &log){
        ui->textEdit->append(LogManager::richText(msgType, log));
    });

    // 点击将编辑框内容打印
    connect(ui->pushButton, &QPushButton::clicked,
            this, [=]{
        const QString text = ui->lineEdit->text();
        // qFatal 表示致命错误，默认处理会报异常的
        switch (ui->comboBox->currentIndex()) {
        case 0: qDebug().noquote() << text; break;
        case 1: qInfo().noquote() << text; break;
        case 2: qWarning().noquote() << text; break;
        case 3: qCritical().noquote() << text; break;
        case 4: qFatal("Test qFatal"); break;
        default: break;
        }

        // 模拟多线程打印
        QtConcurrent::run([](){
            qDebug() << "QtConcurrent::run do something.";
        });
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

