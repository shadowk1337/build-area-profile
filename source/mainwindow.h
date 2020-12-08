#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <usefuldata.h>
#include <QMainWindow>
#include "../Qcustomplot/qcustomplot.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget * = nullptr);
  ~MainWindow();

 protected:
  bool setupFile(void);
  void setupArc(QCustomPlot *);
  void setupAxis(QCustomPlot *);
  void setupCurve(QCustomPlot *);
  void intervalType(QCustomPlot *);

 private:
  Ui::MainWindow *ui;
  QCustomPlot *customPlot;
  QCPItemText *textItem;

 private slots:
  void onMouseMove(QMouseEvent *event);
};
#endif  // MAINWINDOW_H
