#include "coordswindow.h"
#include "ui_coordswindow.h"

struct CoordsWindow::Private {
  Private(double n, double l, double r) : _number(n), _left(l), _right(r){};

  double _number = 0, _left = 0, _right = 0;
};

CoordsWindow::CoordsWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::CoordsWindow) {
  ui->setupUi(this);

  setWindowTitle(tr("РРС1"));
  int w = 320;
  int h = 450;
  setFixedSize(w, h);
}

CoordsWindow::~CoordsWindow() { delete ui; }
