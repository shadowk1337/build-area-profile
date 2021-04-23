#include "diagramwindow.h"
#include "ui_diagramwindow.h"

DiagramWindow::DiagramWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::DiagramWindow) {
  ui->setupUi(this);

  setWindowTitle(tr("Диаграммы уровней передачи"));
  int w = 1380;
  int h = 430;
  const QRect &r = QGuiApplication::primaryScreen()->availableGeometry();
  setGeometry(qAbs(r.width() - w) / 2, qAbs(r.height() - h) / 2, w, h);

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
  drawGraph(ui->customplot_1, C(fromVtToDbvt(_c->data->spec.p.first)),
            C(_c->data->tower.wf.first), C(_c->data->tower.c.first),
            C(_c->data->tower.c.second), _c->data->log_p.first);

  drawGraph(ui->customplot_2, C(fromVtToDbvt(_c->data->spec.p.second)),
            C(_c->data->tower.wf.second), C(_c->data->tower.c.second),
            C(_c->data->tower.c.first), _c->data->log_p.second);
}

void DiagramWindow::setupGraph() {
  ui->customplot_1->yAxis->setVisible(true);
  ui->customplot_1->yAxis->setTickLength(0);
  ui->customplot_1->yAxis->setSubTickLength(0);
  ui->customplot_1->yAxis2->setVisible(true);
  ui->customplot_1->yAxis2->setTickLength(0);
  ui->customplot_1->yAxis2->setSubTickLength(0);

  ui->customplot_2->yAxis->setVisible(true);
  ui->customplot_2->yAxis->setTickLength(0);
  ui->customplot_2->yAxis->setSubTickLength(0);
  ui->customplot_2->yAxis2->setVisible(true);
  ui->customplot_2->yAxis2->setTickLength(0);
  ui->customplot_2->yAxis2->setSubTickLength(0);
}

void DiagramWindow::drawGraph(QCustomPlot *cp, double sp, double wf, double c1,
                              double c2, double log_p) {
  QSharedPointer<GraphPainter> gr = QSharedPointer<GraphPainter>::create(cp);

  QVector<double> x(6), y(6);
  QSharedPointer<QCPAxisTickerText> textTicker_l(new QCPAxisTickerText),
      textTicker_r(new QCPAxisTickerText);

  x[0] = 0, x[2] = x[1] = 1;
  x[4] = x[3] = x[2] + 6;
  x[5] = x[4] + 1;

  y[0] = sp;
  y[1] = y[0] - wf;
  y[2] = y[1] + c1;
  y[3] = y[2] - C(_c->data->ws);
  y[4] = y[3] + c2;
  y[5] = log_p + C(_c->data->wa) + C(_c->data->wp);

  textTicker_r->addTick(y[0], tr("P1"));

  cp->clearGraphs();
  _c->data->gr->draw(x, y, "", QPen(Qt::blue, 2));

  textTicker_r->addTick(y[5], tr("P2'"));
  textTicker_r->addTick(y[3], tr("Wсв"));

  y[3] -= C(_c->data->wp), y[4] = y[3] + c2, y[5] -= C(_c->data->wp);
  _c->data->gr->draw(x, y, "", QPen(Qt::red, 2));

  textTicker_r->addTick(y[5], tr("P2"));
  textTicker_r->addTick(y[3], QString(tr("Wсв + Wр")));

  //  std::cerr << log_p << ' ' << C(_c->data->spec.q) << " | ";

  y[3] -= C(_c->data->spec.q), y[4] = y[3] + c2,
                               y[5] = log_p - C(_c->data->spec.q);
  _c->data->gr->draw(x, y, "", QPen(Qt::black, 2, Qt::DashLine));

  textTicker_r->addTick(y[5], tr("Pпор"));
  textTicker_r->addTick(y[3], tr("Wсв + Wр + Wз"));

  auto max = std::max(y[0], std::max(y[1], y[2]));
  auto min = std::min(y[3], std::min(y[4], y[5]));

  cp->rescaleAxes();
  cp->yAxis->setRange(min - qAbs(.1 * min), max + qAbs(.1 * max));
  cp->yAxis2->setRange(min - qAbs(.1 * min), max + qAbs(.1 * max));

  for (auto it = min; it < max; it += 0.1 * (max - min)) {
    textTicker_l->addTick(it, QString::number(C_REVERSE(it), 'g', 2));
  }

  cp->yAxis->setTicker(textTicker_l);
  cp->yAxis2->setTicker(textTicker_r);
}
