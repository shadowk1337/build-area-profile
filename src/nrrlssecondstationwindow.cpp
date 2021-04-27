#include "nrrlssecondstationwindow.h"
#include "ui_secondstationwindow.h"

SecondStationWindow::SecondStationWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::SecondStationWindow)
{
  ui->setupUi(this);

  setWindowTitle(tr("РРС2"));
  int w = 700;
  int h = 500;
  setFixedSize(w, h);
}

SecondStationWindow::~SecondStationWindow()
{
  delete ui;
}
