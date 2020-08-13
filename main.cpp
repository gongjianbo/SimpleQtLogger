#include <QApplication>

#include "LogManager.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    LogManager::getInstance(); //初始化一下

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
