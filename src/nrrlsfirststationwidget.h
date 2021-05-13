#ifndef NRRLSFIRSTSTATIONWIDGET_H
#define NRRLSFIRSTSTATIONWIDGET_H

#include <QWidget>
#include "nrrlscalc.h"

namespace Ui {
class NRrlsFirstStationWidget;
}

class NRrlsFirstStationWidget : public QWidget {
  Q_OBJECT

 public:
  explicit NRrlsFirstStationWidget(QSharedPointer<NRrls::Calc::Core> c,
                                   QWidget *parent = nullptr);
  ~NRrlsFirstStationWidget();

 private:
  void setConnects();
  void capacityNotNull() const;

 private slots:
  void onChangeHeight(double d);

 private:
  QSharedPointer<NRrls::Calc::Core> _c;
  Ui::NRrlsFirstStationWidget *ui;
};

#endif  // NRRLSFIRSTSTATIONWIDGET_H
