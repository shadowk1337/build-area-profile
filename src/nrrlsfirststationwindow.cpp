#include "nrrlsfirststationwindow.h"
#include "ui_firststationwindow.h"

FirstStationWindow::FirstStationWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::FirstStationWindow)
{
  ui->setupUi(this);

  setWindowTitle(tr("РРС1"));
  int w = 700;
  int h = 500;
  setFixedSize(w, h);
}

FirstStationWindow::~FirstStationWindow()
{
  delete ui;
}
