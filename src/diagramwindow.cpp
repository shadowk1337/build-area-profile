#include "diagramwindow.h"
#include "ui_diagramwindow.h"

DiagramWindow::DiagramWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::DiagramWindow) {
  ui->setupUi(this);

  setWindowTitle(tr("Диаграммы уровней передачи"));
  int w = 1170;
  int h = 430;
  setFixedSize(w, h);

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

void DiagramWindow::init(QSharedPointer<NRrls::Calc::Core> c) { _c = c; }

void DiagramWindow::exec() {
  drawGraph(ui->customplot_1, _c->data->spec.p.first, _c->data->tower.wf.first,
            _c->data->tower.c.first, _c->data->tower.c.second,
            _c->data->p.first);

  drawGraph(ui->customplot_2, _c->data->spec.p.second,
            _c->data->tower.wf.second, _c->data->tower.c.second,
            _c->data->tower.c.first, _c->data->p.second);
}

void DiagramWindow::setupGraph() {
  ui->customplot_1->yAxis->setVisible(true);
  ui->customplot_1->yAxis->setTickLength(0);
  ui->customplot_1->yAxis->setLabel(QObject::tr("P, дБ"));
  ui->customplot_1->yAxis->setSubTickLength(0);
  ui->customplot_1->yAxis2->setVisible(true);
  ui->customplot_1->yAxis2->setTickLength(0);
  ui->customplot_1->yAxis2->setSubTickLength(0);

  ui->customplot_2->yAxis->setVisible(true);
  ui->customplot_2->yAxis->setTickLength(0);
  ui->customplot_2->yAxis->setLabel(QObject::tr("P, дБ"));
  ui->customplot_2->yAxis->setSubTickLength(0);
  ui->customplot_2->yAxis2->setVisible(true);
  ui->customplot_2->yAxis2->setTickLength(0);
  ui->customplot_2->yAxis2->setSubTickLength(0);
}

void DiagramWindow::drawGraph(QCustomPlot *cp, double sp, double wf, double c1,
                              double c2, double p) {
  QVector<double> x(6), y(6);

  x[0] = 0, x[2] = x[1] = .1 * _c->data->constant.area_length;
  x[4] = x[3] = x[2] + _c->data->constant.area_length;
  x[5] = x[4] + .1 * _c->data->constant.area_length;

  y[0] = sp;
  y[1] = y[0] - wf;
  y[2] = y[1] + c1;
  y[3] = y[2] - _c->data->ws;
  y[4] = y[3] + c2;
  y[5] = p + _c->data->wa + _c->data->wp;

  cp->clearGraphs();
  cp->addGraph();
  cp->graph(0)->setPen(QPen(Qt::blue, 2));
  cp->graph(0)->addData(x, y);

  y[3] -= _c->data->wp, y[4] = y[3] + c2, y[5] -= _c->data->wp;
  cp->addGraph();
  cp->graph(1)->setPen(QPen(Qt::red, 2));
  cp->graph(1)->addData(x, y);

  y[3] -= _c->data->wa, y[4] = y[3] + c2, y[5] = p;
  cp->addGraph();
  cp->graph(2)->setPen(QPen(Qt::black, 2));
  cp->graph(2)->setPen(QPen(Qt::DashLine));
  cp->graph(2)->addData(x, y);
  cp->rescaleAxes();
  cp->yAxis2->rescale();
}
