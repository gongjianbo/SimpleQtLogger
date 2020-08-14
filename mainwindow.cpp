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
    connect(LogManager::getInstance(),&LogManager::newLog,ui->textEdit,&QTextEdit::append);

    //点击将编辑框内容打印
    connect(ui->pushButton,&QPushButton::clicked,[=]{
        const QString text=ui->lineEdit->text();
        //qFatal为致命错误，这里没处理
        switch (ui->comboBox->currentIndex()) {
        case 0: qDebug()<<"[debug]"<<text; break;
        case 1: qInfo()<<"[info]"<<text; break;
        case 2: qWarning()<<"[warning]"<<text; break;
        case 3: qCritical()<<"[critical]"<<text; break;
        default:
            break;
        }

        QtConcurrent::run([](){
            qDebug()<<"QtConcurrent::run"<<QDateTime::currentDateTime();
        });
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

