#include "nrrlscoordswindow.h"
#include <iostream>
#include "ui_nrrlscoordswindow.h"

struct NRrlsCoordsWindow::Private {
  Ui::NRrlsCoordsWindow *ui;
};

NRrlsCoordsWindow::NRrlsCoordsWindow(QWidget *parent)
    : QMainWindow(parent), _d(new Private) {
  _d->ui = new Ui::NRrlsCoordsWindow;
  _d->ui->setupUi(this);

  setWindowTitle(tr("Информация"));
  int w = 320;
  int h = 310;
  setFixedSize(w, h);
}

void NRrlsCoordsWindow::init(double l, double r, double hl, double hv,
                             double he, double h, double hn) {
  _d->ui->label_5->setText(QString::number(l));
  _d->ui->label_6->setText(QString::number(r));
  _d->ui->label_8->setText(QString::number(hl));
  _d->ui->label_10->setText(QString::number(hv));
  _d->ui->label_12->setText(QString::number(he));
  _d->ui->label_14->setText(QString::number(h));
  _d->ui->label_16->setText(QString::number(hn));
}

NRrlsCoordsWindow::~NRrlsCoordsWindow() { delete _d->ui; }
