#ifndef DIAGRAMWINDOW_H
#define DIAGRAMWINDOW_H

#include "nrrlscalc.h"
#include <QMainWindow>

namespace Ui {
class NRrlsDiagramWindow;
}

class NRrlsDiagramWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit NRrlsDiagramWindow(QWidget *parent = nullptr);
  ~NRrlsDiagramWindow();

  // Заполнение структуры данными
  void init(QSharedPointer<NRrls::Calc::Core> c);

  void exec();

private:
  // Настройка графика
  void setupGraph();

private:
  void drawGraph(QCustomPlot *cp, double sp, double wf1, double c1,
                 double c2, double log_p);

  Ui::NRrlsDiagramWindow *ui;

private:
  QSharedPointer<NRrls::Calc::Core> _c;
};

#endif // DIAGRAMWINDOW_H
