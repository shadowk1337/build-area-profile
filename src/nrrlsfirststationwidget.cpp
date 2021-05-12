#include "nrrlsfirststationwidget.h"
#include "ui_nrrlsfirststationwidget.h"

NRrlsFirstStationWidget::NRrlsFirstStationWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::NRrlsFirstStationWidget)
{
  ui->setupUi(this);

  setWindowTitle(tr("РРС 1"));
}

NRrlsFirstStationWidget::~NRrlsFirstStationWidget()
{
  delete ui;
}
