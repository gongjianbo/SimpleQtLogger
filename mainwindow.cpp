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

    //关联log信息，显示到文本框
    //注意，如果槽函数为lambda或者其他没有接收者的情况，需要保证槽函数中的变量有效性
    //因为static变量的生命周期更长，可能槽函数所在模块已经释放资源，最好connect加上接收者
    connect(LogManager::getInstance(),&LogManager::newLog,
            this,[this](int msgType, const QString &log){
        QString log_text;
         QTextStream stream(&log_text);
        switch (msgType) {
        case QtDebugMsg: stream<<"<span style='color:green;'>"; break;
        case QtInfoMsg: stream<<"<span style='color:blue;'>"; break;
        case QtWarningMsg: stream<<"<span style='color:yellow;'>"; break;
        case QtCriticalMsg: stream<<"<span style='color:red;'>"; break;
        case QtFatalMsg: stream<<"<span style='color:red;'>"; break;
        default: stream<<"<span style='color:red;'>"; break;
        }
        stream<<log<<"</span>";
        ui->textEdit->append(log_text);
    });

    //点击将编辑框内容打印
    connect(ui->pushButton,&QPushButton::clicked,[=]{
        const QString text=ui->lineEdit->text();
        //qFatal表示致命错误，默认处理会报异常的
        switch (ui->comboBox->currentIndex()) {
        case 0: qDebug()<<text; break;
        case 1: qInfo()<<text; break;
        case 2: qWarning()<<text; break;
        case 3: qCritical()<<text; break;
        case 4: qFatal("Test qFatal"); break;
        default:
            break;
        }

        QtConcurrent::run([](){
            qDebug()<<"QtConcurrent::run at other thread... ...";
        });
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

