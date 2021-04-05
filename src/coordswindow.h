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

 private:
  Ui::CoordsWindow *ui;

 private:
  struct Private;
};

#endif  // COORDSWINDOW_H
