#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  MainWindow w;
  w.setWindowTitle("Area profile");
  QRect rec = QApplication::desktop()->screenGeometry();
  w.move(rec.width() * .1, rec.height() * .2);
  w.show();
  return a.exec();
}
