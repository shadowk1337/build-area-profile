#ifndef DIAGRAMWINDOW_H
#define DIAGRAMWINDOW_H

#include "nrrlscalc.h"
#include <QMainWindow>

namespace Ui {
class DiagramWindow;
}

class DiagramWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit DiagramWindow(QWidget *parent = nullptr);
  ~DiagramWindow();

  // Заполнение структуры данными
  void init(QSharedPointer<NRrls::Calc::Core> c);

  void exec();

private:
  // Настройка графика
  void setupGraph();

private:
  void drawGraph(QCustomPlot *cp, double sp, double wf1, double c1,
                 double c2, double log_p, double s);

  Ui::DiagramWindow *ui;

private:
  QSharedPointer<NRrls::Calc::Core> _c;
};

#endif // DIAGRAMWINDOW_H
