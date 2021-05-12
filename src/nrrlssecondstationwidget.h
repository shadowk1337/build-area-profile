#ifndef NRRLSSECONDSTATIONWIDGET_H
#define NRRLSSECONDSTATIONWIDGET_H

#include <QWidget>

namespace Ui {
  class NRrlsSecondStationWidget;
}

class NRrlsSecondStationWidget : public QWidget
{
  Q_OBJECT

public:
  explicit NRrlsSecondStationWidget(QWidget *parent = nullptr);
  ~NRrlsSecondStationWidget();

private:
  Ui::NRrlsSecondStationWidget *ui;
};

#endif // NRRLSSECONDSTATIONWIDGET_H
