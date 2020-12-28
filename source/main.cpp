#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    try{
      MainWindow w;
      w.setWindowTitle("Area profile");
      w.show();
    }
    catch (...){
      qDebug() << "Smth wrong";
    }

    return a.exec();
}
