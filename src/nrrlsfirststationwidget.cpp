#include "nrrlsfirststationwidget.h"
#include "ui_nrrlsfirststationwidget.h"

NRrlsFirstStationWidget::NRrlsFirstStationWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::NRrlsFirstStationWidget) {}

NRrlsFirstStationWidget::~NRrlsFirstStationWidget() { delete ui; }
