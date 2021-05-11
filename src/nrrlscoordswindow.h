#ifndef COORDSWINDOW_H
#define COORDSWINDOW_H

#include <QMainWindow>

namespace Ui {
class NRrlsCoordsWindow;
}

class NRrlsCoordsWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit NRrlsCoordsWindow(QWidget *parent = nullptr);
  ~NRrlsCoordsWindow();

 public:
  void init(double l, double r, double hl, double hv, double he,
            double h, double hn);

 private:
  struct Private;
  Private *const _d;
};

#endif  // COORDSWINDOW_H
