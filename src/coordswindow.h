#ifndef COORDSWINDOW_H
#define COORDSWINDOW_H

#include <QMainWindow>

namespace Ui {
class CoordsWindow;
}

class CoordsWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit CoordsWindow(QWidget *parent = nullptr);
  ~CoordsWindow();

 public:
  void init(double n, double l, double r, double hl, double hv, double he,
            double h, double hn);

 private:
  struct Private;
  Private *const _d;
};

#endif  // COORDSWINDOW_H
