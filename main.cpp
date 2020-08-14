#include <QApplication>

#include "LogManager.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    LogManager::getInstance()->initManager();//初始化

    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
