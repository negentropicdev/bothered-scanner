#include "mainwindow.h"

#include <QApplication>

#include "qmat.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<QMat>();

    MainWindow w;
    w.show();

    return a.exec();
}
