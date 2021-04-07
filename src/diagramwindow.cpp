#include "diagramwindow.h"
#include "ui_diagramwindow.h"

DiagramWindow::DiagramWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::DiagramWindow) {
  ui->setupUi(this);

  ui->customplot_1->xAxis->setVisible(0);
  ui->customplot_1->yAxis->setVisible(0);
  ui->customplot_2->xAxis->setVisible(0);
  ui->customplot_2->yAxis->setVisible(0);

  QCPTextElement *title = new QCPTextElement(ui->customplot_1);
  title->setText(
      tr("Диаграмма уровней распределения сигнала на интервале РРС1 - РРС2"));
  title->setFont(QFont("FreeSans", 10, QFont::Bold));
  ui->customplot_1->plotLayout()->insertRow(0);
  ui->customplot_1->plotLayout()->addElement(0, 0, title);

  title = new QCPTextElement(ui->customplot_2);
  title->setText(
      tr("Диаграмма уровней распределения сигнала на интервале РРС2 - РРС1"));
  title->setFont(QFont("FreeSans", 10, QFont::Bold));
  ui->customplot_2->plotLayout()->insertRow(0);
  ui->customplot_2->plotLayout()->addElement(0, 0, title);

  setupGraph();
}

DiagramWindow::~DiagramWindow() { delete ui; }

void DiagramWindow::setupGraph() {
  ui->customplot_1->yAxis->setVisible(true);
  ui->customplot_1->yAxis2->setVisible(true);
  ui->customplot_1->yAxis->setTickLength(0);
  ui->customplot_1->yAxis->setLabel(QObject::tr("P, дБ"));
  ui->customplot_1->yAxis2->setTickLength(0);
  ui->customplot_1->yAxis->setSubTickLength(0);
  ui->customplot_1->yAxis2->setSubTickLength(0);

  ui->customplot_2->yAxis->setVisible(true);
  ui->customplot_2->yAxis2->setVisible(true);
  ui->customplot_2->yAxis->setTickLength(0);
  ui->customplot_2->yAxis->setLabel(QObject::tr("P, дБ"));
  ui->customplot_2->yAxis2->setTickLength(0);
  ui->customplot_2->yAxis->setSubTickLength(0);
  ui->customplot_2->yAxis2->setSubTickLength(0);
}
