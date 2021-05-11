#ifndef NRRLSFIRSTSTATIONWIDGET_H
#define NRRLSFIRSTSTATIONWIDGET_H

#include <QWidget>

namespace Ui {
  class NRrlsFirstStationWidget;
}

class NRrlsFirstStationWidget : public QWidget
{
  Q_OBJECT

public:
  explicit NRrlsFirstStationWidget(QWidget *parent = nullptr);
  ~NRrlsFirstStationWidget();

private:
  Ui::NRrlsFirstStationWidget *ui;
};

#endif // NRRLSFIRSTSTATIONWIDGET_H
