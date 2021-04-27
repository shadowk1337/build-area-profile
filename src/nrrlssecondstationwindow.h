#ifndef SECONDSTATIONWINDOW_H
#define SECONDSTATIONWINDOW_H

#include <QMainWindow>

namespace Ui {
  class SecondStationWindow;
}

class SecondStationWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit SecondStationWindow(QWidget *parent = nullptr);
  ~SecondStationWindow();

private:
  Ui::SecondStationWindow *ui;
};

#endif // SECONDSTATIONWINDOW_H
