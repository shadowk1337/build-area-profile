#include "coordswindow.h"
#include "ui_coordswindow.h"

struct CoordsWindow::Private {
  Ui::CoordsWindow *ui;
};

CoordsWindow::CoordsWindow(QWidget *parent)
    : QMainWindow(parent), _d(new Private) {
  _d->ui = new Ui::CoordsWindow;
  _d->ui->setupUi(this);

  setWindowTitle(tr("Координаты"));
  int w = 320;
  int h = 450;
  setFixedSize(w, h);
}

void CoordsWindow::init(double n, double l, double r) {
  _d->ui->label_2->setText(QString::number(n));
  _d->ui->label_5->setText(QString::number(l));
  _d->ui->label_6->setText(QString::number(r));
}

CoordsWindow::~CoordsWindow() { delete _d->ui; }
