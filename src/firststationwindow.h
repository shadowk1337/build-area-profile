#ifndef FIRSTSTATIONWINDOW_H
#define FIRSTSTATIONWINDOW_H

#include <QMainWindow>

namespace Ui {
class FirstStationWindow;
}

class FirstStationWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit FirstStationWindow(QWidget *parent = nullptr);
  virtual ~FirstStationWindow();

 private:
  Ui::FirstStationWindow *ui = nullptr;
};

#endif  // FIRSTSTATIONWINDOW_H
