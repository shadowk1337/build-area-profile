#ifndef DIAGRAMWINDOW_H
#define DIAGRAMWINDOW_H

#include <QMainWindow>

namespace Ui {
class DiagramWindow;
}

class DiagramWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit DiagramWindow(QWidget *parent = nullptr);
  ~DiagramWindow();

  void exec();

 private:
  void setupGraph();

 private:
  Ui::DiagramWindow *ui;

 private:
//  double _
};

#endif  // DIAGRAMWINDOW_H
