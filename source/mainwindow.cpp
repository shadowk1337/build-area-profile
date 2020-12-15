#include "mainwindow.h"
#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <map>
#include <new>
#include "calcformulas.h"
#include "constants.h"
#include "datastruct.h"
#include "intervals.h"
#include "ui_mainwindow.h"

Data data_usage;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  assert(setupFile() == true);
  setupArc(ui->customPlot);
  setupAxis(ui->customPlot);
  setupCurve(ui->customPlot);
  intervalType(ui->customPlot);

  textItem = new QCPItemText(ui->customPlot);
  connect(ui->customPlot, &QCustomPlot::mouseMove, this,
          &MainWindow::onMouseMove);
  ui->customPlot->replot();
}

MainWindow::~MainWindow() { delete ui, delete customPlot, delete textItem; }

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

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Could't open the data file";
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
    data_usage.s_heights.push_back(line.section(";", 3, 3).toDouble());
    data_usage.s_counter++;
  }

  assert(data_usage.s_heights.size() <= data_usage.s_counter);
  file.close();

  if (!data_usage.s_counter) {
    qDebug() << "File is empty";
    return false;
  }

  data_usage.s_intervals_difference =
      constants::AREA_LENGTH / data_usage.s_counter;
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
  QVector<qreal> x(constants::AREA_LENGTH), y(constants::AREA_LENGTH);
  bool first = true;
  qreal move_graph_up_value;
  qreal equivalent_radius = equivalentRadius(constants::G_STANDARD);
  qreal s_intervals_difference = constants::AREA_LENGTH / data_usage.s_counter;

  for (qreal i = -constants::R_EVALUATED, iterator = 0;
       i < constants::R_EVALUATED;
       i += s_intervals_difference, iterator += s_intervals_difference) {
    x[i + constants::R_EVALUATED] = i + constants::R_EVALUATED;
    if (first) {
      move_graph_up_value = qAbs(deltaY(i, equivalent_radius));
      y[i + constants::R_EVALUATED] = 0;
      first = false;
      data_usage.s_map[iterator] = y[i + constants::R_EVALUATED];
    } else {
      y[i + constants::R_EVALUATED] =
          -deltaY(i, equivalent_radius) + move_graph_up_value;
      data_usage.s_map[iterator] = y[i + constants::R_EVALUATED];
    }
  }
  arcPlot->addGraph();
  arcPlot->graph(0)->setData(x, y);
  arcPlot->graph(0)->setPen(QPen(QColor("#014506")));
  QCPItemTracer *tracer = new QCPItemTracer(arcPlot);
  tracer->setGraph(arcPlot->graph(0));
  tracer->updatePosition();
  assert(x.size() == y.size());
}

void lineOfSight(QCustomPlot *linePlot);

// Кривая высот
void MainWindow::setupCurve(QCustomPlot *curvPlot) {
  QVector<qreal> heights = data_usage.s_heights;
  QVector<qreal> x(data_usage.s_counter - 1), y(data_usage.s_counter - 1);
  qreal i;
  qint32 v;

  for (qreal i = 0, v = 0; v < data_usage.s_counter;
       i += data_usage.s_intervals_difference, v++)
    heights[v] += data_usage.s_map[i];

  for (i = 0, v = 0; v + 1 < data_usage.s_counter;
       i += data_usage.s_intervals_difference, v++) {
    x[v] = i;
    y[v] = heights[v];
  }

  assert(x.size() == y.size());
  curvPlot->addGraph();
  curvPlot->graph(1)->setData(x, y);
  curvPlot->graph(1)->setPen(QPen(QColor("#137ea8")));
  lineOfSight(curvPlot);
  data_usage.s_heights = heights;  // H + h map
}

// Линия прямой видимости
void lineOfSight(QCustomPlot *linePlot) {
  QVector<qreal> x(data_usage.s_counter), y(data_usage.s_counter);
  qreal x_start = 0;
  qreal y_start = 117.49;
  qreal x_end = 33700;
  qreal y_end = 52.7;
  qreal x_diff = (x_end - x_start) / data_usage.s_counter;
  qreal y_diff = (y_end - y_start) / data_usage.s_counter;

  for (qint32 i = 0; i < data_usage.s_counter; i++) {
    x[i] = x_start + i * x_diff;
    y[i] = y_start + i * y_diff;
  }

  linePlot->addGraph();
  linePlot->graph(2)->setData(x, y);
  linePlot->graph(2)->setPen(QPen(QColor("#d6ba06")));
}

void intervalTypeCalc(QCustomPlot *customPlot,
                      const QVector<qint32> &interval_type);

void typeDefinition(const QVector<qreal> &H, const QVector<qreal> &HNull,
                    const QVector<qreal> &hNull,
                    QVector<qint32> &interval_type) {
  assert(HNull.size() == hNull.size());

  for (qint32 i = 0; i < H.size(); ++i) {
    if (H.at(i) >= HNull.at(i) && hNull.at(i) >= 0)
      interval_type[i] = 1;
    else if (HNull.at(i) > H.at(i) && H.at(i) > 0.1 && hNull.at(i) < 0.1 &&
             hNull.at(i) > 0)
      interval_type[i] = 2;
    else if (HNull.at(i) > H.at(i) && hNull.at(i) < 0)
      interval_type[i] = 3;
    else
      interval_type[i] = 1;
  }
}

void MainWindow::intervalType(QCustomPlot *customPlot) {
  customPlot->addGraph();
  QVector<qreal> curve_heights = data_usage.s_heights;
  QVector<qreal> line_of_sight_heights(data_usage.s_counter),
      H(data_usage.s_counter), HNull(data_usage.s_counter),
      hNull(data_usage.s_counter), it_x_vector, it_y_vector, to_save;
  QVector<qint32> interval_type(
      data_usage.s_counter);  // 1 - Открытый, 2 - Полуоткрытый, 3 - Закрытый
  for (qint32 i = 0; i < data_usage.s_counter; ++i) {
    line_of_sight_heights[i] =
        customPlot->graph(2)
            ->dataValueRange(i * (data_usage.s_intervals_difference / 100))
            .center();
    H[i] = line_of_sight_heights[i] - curve_heights[i];
    HNull[i] =
        sqrt(constants::AREA_LENGTH * constants::LAMBDA *
             k_from_R(i * data_usage.s_intervals_difference) *
             ((1 - k_from_R(i * data_usage.s_intervals_difference)) / 3));
    hNull[i] = H[i] / HNull[i];
    it_x_vector.push_back(i * data_usage.s_intervals_difference);
    it_y_vector.push_back(-HNull.at(i) + line_of_sight_heights[i]);
    typeDefinition(H, HNull, hNull, interval_type);
  }
  assert(HNull.size() == hNull.size());
  for (auto i = 0; i < hNull.size(); ++i) {
    to_save.push_back(HNull[i] / hNull[i]);
  }
  data_usage.s_HNull_hNull_div =
      to_save;  //  вектор со значениями hNull / hNull ---> save вектор
  data_usage.s_hNull = hNull;
  data_usage.s_HNull = HNull;
  data_usage.s_H = H;
  intervalTypeCalc(customPlot, interval_type);

  customPlot->addGraph();                                   // для теста
  customPlot->graph(3)->setData(it_x_vector, it_y_vector);  //

  std::for_each(it_y_vector.begin(), it_y_vector.end(), [&](qreal &x) {  //
    static qint32 i = 0;                                                 //
    x += 2 * HNull[i];                                                   //
    i++;                                                                 //
  });                                                                    //
  customPlot->graph(4)->setData(it_x_vector, it_y_vector);               //
}

QVector<qreal> findIntersectionXCoord(const QVector<QPair<QPointF, QPointF>> &,
                                      const QVector<QPair<QPointF, QPointF>> &);

// Определение типа интервала по расстоянию от начала отсчета
void intervalTypeCalc(QCustomPlot *customPlot,
                      const QVector<qint32> &interval_type) {
  QVector<qint32> first_type_indexes, second_type_indexes, third_type_indexes;
  for (qint32 i = 0; i < interval_type.size(); ++i) {
    if (interval_type.at(i) == 1)
      first_type_indexes.push_back(i);
    else if (interval_type.at(i) == 2)
      second_type_indexes.push_back(i);
    else
      third_type_indexes.push_back(i);
  }
  if (!first_type_indexes.empty()) {
    Interval *i = new OpenedInterval(first_type_indexes);
    i->IntervalType(customPlot, first_type_indexes);
  }
  if (!second_type_indexes.empty()) {
    Interval *i = new HalfOpenedInterval(second_type_indexes);
    i->IntervalType(customPlot, second_type_indexes);
  }
  if (!third_type_indexes.empty()) {
    Interval *i = new ClosedInterval(third_type_indexes);
    i->IntervalType(customPlot, third_type_indexes);
  }
}

inline qreal l0(qreal h0, qreal k) {
  return ((constants::AREA_LENGTH)*qSqrt(1 + h0 * h0)) /
         (1 + (h0 * h0) / (4 * k * (1 - k)));
}

//-------------------------------------------_-------------------------------------------

// Нахождение точки пересечения двух линий, заданных двумя точками
QVector<qreal> findIntersectionXCoord(
    const QVector<QPair<QPointF, QPointF>> &making_lines_points,
    const QVector<QPair<QPointF, QPointF>> &land_lines_points) {
  assert(making_lines_points.size() == land_lines_points.size());
  QVector<qreal> x_coords;
  for (qint32 i = 0; i < making_lines_points.size(); ++i) {
    qreal der1 =
        (making_lines_points[i].second.y() - making_lines_points[i].first.y()) /
        (making_lines_points[i].second.x() - making_lines_points[i].first.x());
    qreal der2 =
        (land_lines_points[i].second.y() - land_lines_points[i].first.y()) /
        (land_lines_points[i].second.x() - land_lines_points[i].first.x());
    assert(der1 != der2);
    x_coords.push_back(((land_lines_points[i].first.y() -
                         land_lines_points[i].first.x() * der2) -
                        (making_lines_points[i].first.y() -
                         making_lines_points[i].first.x() * der1)) /
                       (der1 - der2));
  }
  return x_coords;
}

/*customPlot->addGraph();                                   // для теста
customPlot->graph(3)->setData(it_x_vector, it_y_vector);  //

std::for_each(it_y_vector.begin(), it_y_vector.end(), [&](qreal &x) {  //
  static qint32 i = 0;                                                 //
  x += 2 * HNull[i];                                                   //
  i++;                                                                 //
});                                                                    //
customPlot->graph(4)->setData(it_x_vector, it_y_vector);               //
*/
