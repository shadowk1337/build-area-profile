#include "mainwindow.h"
#include <algorithm>
#include <cassert>
#include <new>
#include "calcformules.h"
#include "constants.h"
#include "datastruct.h"
#include "intervals.h"
#include "ui_mainwindow.h"

using namespace EarthApprox;

Data *s_data = new Data;
SenRecCoords *s_tower_coords = new SenRecCoords;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  assert(setupFile() == true);
  setupAxis(ui->customPlot);
  setupArc(ui->customPlot);
  setupCurve(ui->customPlot);
  dataInit();
  fresnelZone(ui->customPlot);
  intervalType(ui->customPlot);

  textItem = new QCPItemText(ui->customPlot);
  connect(ui->customPlot, &QCustomPlot::mouseMove, this,
          &MainWindow::onMouseMove);
  ui->customPlot->replot();
}

MainWindow::~MainWindow() {
  delete ui, delete customPlot, delete textItem;
  delete s_data;
  delete s_tower_coords;
}

void MainWindow::onMouseMove(QMouseEvent *event) {
  QCustomPlot *customPlot = qobject_cast<QCustomPlot *>(sender());
  qreal x = customPlot->xAxis->pixelToCoord(event->pos().x());
  qreal y = customPlot->yAxis->pixelToCoord(event->pos().y());
  textItem->setText(QString("(%1, %2)")
                        .arg(static_cast<qint32>(x) / 1000)
                        .arg(static_cast<qint32>(y)));
  textItem->position->setCoords(QPointF(x, y));
  customPlot->replot();
}

// Чтение данных из файла с высотами
bool MainWindow::setupFile(void) {
  QFile file("heights.csv");
  QTextStream in(&file);
  QRegExp rx("[ ;]");
  size_t count = 0;

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Could't open the s_data file";
    return false;
  }

  bool first = true;

  while (!in.atEnd()) {
    QString line = in.readLine();
    if (first) {
      first = false;
      continue;
    }
    line.replace(",", ".");
    s_data->indexes.push_back(count++);
    s_data->heights.push_back(line.section(";", 3, 3).toDouble());
  }

  assert(s_data->heights.size() <= s_data->indexes.size());
  file.close();

  if (!count) {
    qDebug() << "File is empty";
    return false;
  }
  s_data->intervals_difference = constants::AREA_LENGTH / count;
  return true;
}

// Настройки осей
void MainWindow::setupAxis(QCustomPlot *axis) {
  QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
  QString str;
  QFont pfont("Arial", 8);

  axis->rescaleAxes();
  axis->yAxis->scaleRange(2);
  axis->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |
                        QCP::iSelectPlottables);

  for (qreal i = 0; i < constants::AREA_LENGTH + 500; i += 500) {
    str = QString::number(static_cast<qint32>(i) / 1000);
    textTicker->addTick(i, str);
  }

  axis->xAxis->setTicker(textTicker);
  axis->xAxis->setTickLabelFont(pfont);
  axis->xAxis->setTickLabelRotation(-45);
  axis->xAxis->setTickLength(0);
  axis->yAxis->setSubTickLength(0);
  axis->xAxis->setRange(0, constants::AREA_LENGTH);
}

// кривая земной поверхности
void MainWindow::setupArc(QCustomPlot *arcPlot) {
  QVector<qreal> x, y;
  bool first = true;
  qreal move_graph_up_value;
  qreal equivalent_radius = equivalentRadius(constants::G_STANDARD);
  QPen pen(QColor("#014506"));
  pen.setWidth(2);

  for (qreal i = -constants::R_EVALUATED, iterator = 0;
       i < constants::R_EVALUATED; i += s_data->intervals_difference,
             iterator += s_data->intervals_difference) {
    x.push_back(i + constants::R_EVALUATED);
    if (first) {
      move_graph_up_value = qAbs(deltaY(i, equivalent_radius));
      y.push_back(0);
      first = false;
    } else
      y.push_back(-deltaY(i, equivalent_radius) + move_graph_up_value);
  }
  arcPlot->addGraph();
  arcPlot->graph(0)->setData(x, y, true);
  arcPlot->graph(0)->setPen(pen);
  QCPItemTracer *tracer = new QCPItemTracer(arcPlot);
  tracer->setGraph(arcPlot->graph(0));
  tracer->updatePosition();
  assert(x.size() == y.size());
}

void lineOfSight(QCustomPlot *linePlot);

// Кривая высот
void MainWindow::setupCurve(QCustomPlot *curvPlot) {
  QVector<qreal> heights = s_data->heights;
  QVector<qreal> x, y;
  qreal i = 0;
  QPen pen(QColor("#137ea8"));
  pen.setWidth(2);

  for (auto h : s_data->heights) {
    x.push_back(i);
    y.push_back(h);
    i += s_data->intervals_difference;
  }

  assert(x.size() == y.size());
  curvPlot->addGraph();
  curvPlot->graph(1)->setData(x, y);
  curvPlot->graph(1)->setPen(pen);
  lineOfSight(curvPlot);
  s_data->heights = heights;  // H + h
}

// Линия прямой видимости
void lineOfSight(QCustomPlot *linePlot) {
  QVector<qreal> y;
  s_tower_coords->x_sender = 0;
  s_tower_coords->y_sender = 117.49;
  s_tower_coords->x_reciever = 33700;
  s_tower_coords->y_reciever = 52.7;
  s_tower_coords->y_diff =
      (s_tower_coords->y_reciever - s_tower_coords->y_sender) /
      s_data->indexes.size();
  QPen pen(QColor("#d6ba06"));
  pen.setWidth(2);

  for (auto ind : s_data->indexes) {
    s_data->los_heights.push_back(s_tower_coords->y_sender +
                                  ind * s_tower_coords->y_diff);
  }

  QCPItemLine *line = new QCPItemLine(linePlot);
  line->setPen(pen);
  line->start->setCoords(s_tower_coords->x_sender, s_tower_coords->y_sender);
  line->end->setCoords(s_tower_coords->x_reciever, s_tower_coords->y_reciever);
}

void MainWindow::dataInit(void) const {
  for (qint32 i = 0; i < s_data->indexes.size(); ++i) {
    s_data->H.push_back(s_data->los_heights.at(i) - s_data->heights.at(i));
    s_data->H_null.push_back(HNull(i));
    s_data->h_null.push_back(s_data->H.at(i) / s_data->H_null[i]);
    s_data->HNull_hNull_div.push_back(s_data->H_null.at(i) /
                                      s_data->h_null.at(i));
  }
  s_data->H_null[s_data->indexes.size() - 1] = 0;
}

// Зона Френеля
void MainWindow::fresnelZone(QCustomPlot *zonePlot) {
  QVector<qreal> it_x_vector, it_y_vector;
  QPen pen(Qt::red);
  pen.setWidth(2);

  for (qint32 i = 0; i + 1 <= s_data->heights.size(); ++i) {
    it_x_vector.push_back(i * s_data->intervals_difference);
    it_y_vector.push_back(-s_data->H_null.at(i) + s_data->los_heights.at(i));
  }
  it_y_vector[s_data->heights.size() - 1] = s_tower_coords->y_reciever;
  zonePlot->addGraph();
  zonePlot->graph(2)->setPen(pen);
  zonePlot->graph(2)->setData(it_x_vector, it_y_vector);
  std::for_each(it_y_vector.begin(), it_y_vector.end() - 1, [&](qreal &x) {
    static qint32 i = 0;
    x += 2 * s_data->H_null.at(i);
    i++;
  });
  it_y_vector[s_data->heights.size() - 1] = s_tower_coords->y_reciever;
  zonePlot->addGraph();
  zonePlot->graph(3)->setPen(pen);
  zonePlot->graph(3)->setData(it_x_vector, it_y_vector);
}

void intervalTypeCalc(QCustomPlot *, qint32);

// Определение типа интервала
qint32 typeDefinition(void) {
  qint32 interval_type = 0;  // 1 - Открытый, 2 - Полуоткрытый, 3 - Закрытый
  for (qint32 i = 0; i < s_data->H.size(); ++i) {
    if (s_data->H.at(i) >= s_data->H_null.at(i) && s_data->h_null.at(i) >= 0)
      interval_type = std::max(interval_type, 1);
    else if (s_data->H_null.at(i) > s_data->H.at(i) && s_data->H.at(i) > 0.1 &&
             s_data->h_null.at(i) < 0.1 && s_data->h_null.at(i) > 0)
      interval_type = std::max(interval_type, 2);
    else if (s_data->H_null.at(i) > s_data->H.at(i) && s_data->h_null.at(i) < 0)
      interval_type = 3;
  }
  return interval_type;
}

void MainWindow::intervalType(QCustomPlot *customPlot) {
  auto interval_type = typeDefinition();

  assert(s_data->H_null.size() == s_data->h_null.size());
  intervalTypeCalc(customPlot, interval_type);
}

// Определение типа интервала по расстоянию от начала отсчета
void intervalTypeCalc(QCustomPlot *customPlot, qint32 type) {
  Interval *i = nullptr;
  if (type == 1) {
    i = new OpenedInterval;
  }
  if (type == 2) {
    i = new HalfOpenedInterval;
  }
  if (type == 3) {
    i = new ClosedInterval;
  }
  i->exec();
}
