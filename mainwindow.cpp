#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "LogManager.h"

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
        case 0: qDebug()<<text; break;
        case 1: qInfo()<<text; break;
        case 2: qWarning()<<text; break;
        case 3: qCritical()<<text; break;
        default:
            break;
        }

        QtConcurrent::run([](){
            qDebug()<<"QtConcurrent::run";
        });
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

