#include "nrrlssecondstationwidget.h"
#include "ui_nrrlssecondstationwidget.h"

NRrlsSecondStationWidget::NRrlsSecondStationWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::NRrlsSecondStationWidget) {
  ui->setupUi(this);

  setWindowTitle(tr("РРС 2"));
}

NRrlsSecondStationWidget::~NRrlsSecondStationWidget() { delete ui; }
