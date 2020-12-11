#include "mainwindow.h"
#include <qcustomplot.h>
//#include <usefuldata.h>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QRegExp>
#include <QTextStream>
#include <QtGlobal>
#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <map>
#include <new>
#include "constants.h"
#include "ui_mainwindow.h"

struct {
  qint32 s_counter;
  qreal s_intervals_difference;
  std::map<qreal, qreal> s_map;
  QVector<qreal> s_heights, s_HNull_hNull_div, s_hNull, s_HNull, s_H;
} data_usage;

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
qreal equivalentRadius(qreal g);
qreal deltaY(qreal r, qreal eq_radius);

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

inline qreal k_from_R(qreal R) { return R / constants::AREA_LENGTH; }

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

#define FUNC_DECL(s) void s(const QVector<qint32> &)

FUNC_DECL(openedInterval);
FUNC_DECL(halfopenedInterval);
// FUNC_DECL(closedInterval);

void closedInterval(QCustomPlot *, const QVector<qint32> &);

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
  if (!first_type_indexes.empty()) openedInterval(first_type_indexes);
  if (!second_type_indexes.empty()) halfopenedInterval(second_type_indexes);
  if (!third_type_indexes.empty())
    closedInterval(customPlot, third_type_indexes);
}

inline qreal l0(qreal h0, qreal k) {
  return ((constants::AREA_LENGTH)*qSqrt(1 + h0 * h0)) /
         (1 + (h0 * h0) / (4 * k * (1 - k)));
}

//-------------------------------------------Открытый_интервал-------------------------------------------

void openedIntervalPlaneApproximation(qint32, qint32, qint32);

void openedIntervalSphereApproximation(qint32, qint32, qint32);

void openedInterval(const QVector<qint32> &interval_type) {
  qint32 idx_interval_start, idx_interval_end, prev, l0_length;
  idx_interval_start = prev = interval_type[0];

  for (auto it : interval_type) {
    if (abs(it - prev) > 1 || it == *(interval_type.end() - 1)) {
      idx_interval_end = prev;
      qint32 difference = idx_interval_end - idx_interval_start;

      for (qint32 i = idx_interval_start + difference / 4;
           i <= idx_interval_end; i += difference / 4 + 1) {
        auto a = i - l0_length / (2 * data_usage.s_intervals_difference);
        auto b = i + l0_length / (2 * data_usage.s_intervals_difference);
        if ((l0_length = l0(data_usage.s_hNull[i],
                            i * data_usage.s_intervals_difference /
                                (constants::AREA_LENGTH))) <=
            0.25 * constants::AREA_LENGTH) {
          openedIntervalPlaneApproximation(
              std::max(static_cast<qint32>(a), 0),
              std::min(static_cast<qint32>(b),
                       static_cast<qint32>(constants::AREA_LENGTH)),
              i);
        } else {
          openedIntervalSphereApproximation(
              std::max(static_cast<qint32>(a), 0),
              std::min(static_cast<qint32>(b),
                       static_cast<qint32>(constants::AREA_LENGTH)),
              i);
        }
      }
      idx_interval_start = it;
    }
    prev = it;
  }
}

// Критерий Рэлея
void rayleighAndGroundCriteria(qint32 line_start, qint32 line_end,
                               qint32 idx_line_mid);

// Открытые интервалы
void openedIntervalPlaneApproximation(qint32 idx_line_start,
                                      qint32 idx_line_end,
                                      qint32 idx_line_mid) {
  QVector<qreal> x, y;
  rayleighAndGroundCriteria(idx_line_start, idx_line_end, idx_line_mid);
}

void openedIntervalSphereApproximation(qint32 line_start, qint32 line_end,
                                       qint32 idx_line_mid) {
  QVector<qreal> H0_h0_div = data_usage.s_HNull_hNull_div;
  auto max_H0_h0 = 0.75 * H0_h0_div.at(idx_line_mid);
  for (auto i = line_start; i < line_end; i++) {
    //    if (H0_h0_div.at(i) < max_H0_h0) qDebug() << "Гладкая";
    //    else
    //      qDebug() << "-";
  }
}

void rayleighAndGroundCriteria(qint32 line_start, qint32 line_end,
                               qint32 idx_line_mid) {
  qreal k, b, denominator, y, delta_h;
  auto heights = data_usage.s_heights;
  denominator = (line_end - line_start);
  k = (heights.at(line_end) -  // TODO
       heights.at(line_start) / denominator);
  b = (line_end * heights.at(line_start) - line_start * heights.at(line_end)) /
      denominator;
  auto max_H0_h0 = 0.75 * data_usage.s_HNull_hNull_div.at(idx_line_mid);
  for (auto i = line_start; i <= line_end; ++i) {
    y = k * i + b;
    delta_h = abs(y - heights.at(i));
    //    if (delta_h < 0.75 * max_H0_h0) qDebug() << i << ": Гладкая";
  }
}

//-------------------------------------------Полуоткрытый_интервал-------------------------------------------

inline qreal obstacleSphereRadius(qreal l0, qreal delta_y) {
  return ((l0 * l0) / (8 * delta_y)) * 0.001;
}

inline qreal areaReliefParameter(qreal k, qreal H0, qreal obstacleShereRadius) {
  return pow(qPow(constants::AREA_LENGTH, 2) * k * k * ((1 - k) * (1 - k)) /
                 (obstacleShereRadius * H0),
             1.0 / 3);
}

inline qreal reliefParFuncSph(qreal mu) { return 4 + 10 / (mu - 0.1); }

inline qreal attenuationPSph(qreal mu) {
  return 6 + 16.4 / (mu * (1 + 0.8 * mu));
}

inline qreal nuWedg(qreal H, qreal k) {
  return -H *
         qSqrt(2 / (constants::LAMBDA * constants::AREA_LENGTH * k * (1 - k)));
}

inline qreal attentuationPWedg(qreal nu) {
  return 6.9 + 20 * log10(qSqrt(pow(nu - 0.1, 2) + 1) + nu - 0.1);
}

qreal findDGDots(qint32, qint32);
std::pair<qreal, qreal> halfopenedIntervalSphereApproximation(qint32, qreal);
qreal halfopenedIntervalWedgeApproximation(qint32);

void halfopenedInterval(
    const QVector<qint32> &interval_type) {  // TODO: добавить аппроксимацию
                                             // двух соседних препятствий
  qint32 idx_interval_start, idx_interval_end, prev;
  idx_interval_start = prev = interval_type[0];
  QVector<qreal> attentuationP;

  for (auto it : interval_type) {
    if (it - prev > 3 ||
        it == *(interval_type.end() -
                1)) {  // допускаем наличие интервалов другого типа между
                       // интервалами полуоткрытого типа
      idx_interval_end = prev;

      qreal a = obstacleSphereRadius(
          (idx_interval_end - idx_interval_start) *
              data_usage.s_intervals_difference,
          findDGDots(idx_interval_start, idx_interval_end));
      qint32 idx_avg = (idx_interval_end + idx_interval_start) / 2;
      if (a >=
          sqrt(constants::AREA_LENGTH * constants::LAMBDA * 0.5 * (0.5 / 3))) {
        halfopenedIntervalSphereApproximation(idx_avg, a);
      } else {
        halfopenedIntervalWedgeApproximation(idx_avg);
      }
      idx_interval_start = it;
    }
    prev = it;
  }
}

std::pair<qreal, qreal> halfopenedIntervalSphereApproximation(
    qint32 idx_avg, qreal obst_sph_radius) {
  qreal k_avg = k_from_R(idx_avg * data_usage.s_intervals_difference);
  qreal mu = areaReliefParameter(k_avg, data_usage.s_HNull.at(idx_avg),
                                 obst_sph_radius);
  return {reliefParFuncSph(mu), attenuationPSph(mu)};
}

qreal halfopenedIntervalWedgeApproximation(qint32 idx_avg) {
  qreal k_avg = k_from_R(idx_avg * data_usage.s_intervals_difference);
  qreal nu = nuWedg(data_usage.s_H.at(idx_avg), k_avg);
  return attentuationPWedg(nu);
}

qreal findDGDots(qint32 first,
                 qint32 last) {  // TODO: сделать считывание из файла
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

  QVector<qint32> intersec_heights;
  for (qint32 i = 0; i < data_usage.s_counter; ++i) {
    auto a = static_cast<qint32>(data_usage.s_heights.at(i));
    auto b = static_cast<qint32>(y.at(i) - data_usage.s_HNull.at(i));
    if (a == b || a + 1 == b || b + 1 == a) intersec_heights.push_back(i);
  }
  if (intersec_heights.size() == 1 || !intersec_heights.size()) {
    qDebug() << "Not enough intersections";
    return 0;
  }
  auto right = *std::lower_bound(intersec_heights.begin(),
                                 intersec_heights.end(), first);
  auto left =
      *std::lower_bound(intersec_heights.rbegin(), intersec_heights.rend(),
                        last, [](qint32 a, qint32 b) { return a > b; });
  if (right == left)
    right = *std::lower_bound(intersec_heights.begin(), intersec_heights.end(),
                              right);
  return abs(data_usage.s_heights.at(right) - data_usage.s_heights.at(left));
}

//-------------------------------------------Закрытый_интервал-------------------------------------------

qint32 findLongestInterval(const QVector<qint32> &v);
void reliefTangentStraightLines(QCustomPlot *, qint32, qint32);

void closedInterval(QCustomPlot *cp, const QVector<qint32> &interval_type) {
  qint32 idx_interval_start, prev = 0;
  idx_interval_start = prev = interval_type[0];
  auto max = findLongestInterval(interval_type) / 2;

  for (auto it : interval_type) {
    if (it - prev >= max && prev == *interval_type.begin())
      idx_interval_start = it;
    if ((it - prev >= max && prev != *interval_type.begin()) ||
        it == *(interval_type.end() - 1)) {
      reliefTangentStraightLines(cp, idx_interval_start, prev);
      idx_interval_start = it;
    }
    prev = it;
  }
}

qint32 findLongestInterval(const QVector<qint32> &v) {
  qint32 a, int_begin = v.at(0), prev = v.at(0), longestSize = 0;
  for (qint32 i = 0; i < v.size(); ++i) {
    if (v.at(i) - prev > 1) {
      a = prev - int_begin;
      longestSize = (a > longestSize) ? a : longestSize;
      int_begin = v.at(i);
    }
    prev = v.at(i);
  }
  return longestSize;
}

inline std::pair<qreal, qreal> strLineEquation(qreal, qreal, qreal, qreal);
// inline std::pair<qreal, qreal> findClosestMaxPoint(qint32 int_curr, qreal x,
// qreal y, qreal a, qreal b);

void reliefTangentStraightLines(QCustomPlot *cp, qint32 int_start,
                                qint32 int_end) {
  qreal dist = data_usage.s_intervals_difference;
  bool is_higher_sender, is_higher_reciever;
  is_higher_sender = is_higher_reciever = 1;

  for (auto i = int_start; i <= int_end; ++i) {
    auto [a_sender, b_sender] =
        strLineEquation(0, 117.49, i * dist, data_usage.s_heights.at(i));
    //    auto [a_reciever, b_reciever] = strLineEquation(
    //        constants::AREA_LENGTH, 52.7, i * dist,
    //        data_usage.s_heights.at(i));
    //    if (!is_higher_sender) {
    for (auto j = i; j <= int_end; ++j) {
      if (a_sender * j * dist + b_sender < data_usage.s_heights.at(j))
        is_higher_sender = 0;
    }
    //    }
    /*if (!is_higher_reciever) {
      is_higher_reciever = 1;
      for (auto k = i; k >= int_start; --k) {
        if (a_reciever * k * dist + b_reciever < data_usage.s_heights.at(k))
          is_higher_reciever = 0;
      }
    }*/

    if (is_higher_sender) {
      qDebug() << "h";
      QCPItemLine *line = new QCPItemLine(cp);  // TODO: QCustomPlot
      line->start->setCoords(0, 117.49);
      line->end->setCoords(i * dist, data_usage.s_heights.at(i));
      return;
    }
    is_higher_sender = 1;
  }
}

// y = ax + b
std::pair<qreal, qreal> strLineEquation(qreal x, qreal x_relief, qreal y,
                                        qreal y_relief) {
  qreal c = (y_relief - y);
  qreal d = (x_relief - x);
  return {c / d, y - (x * c / d)};
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

qreal equivalentRadius(qreal g) {
  return constants::REAL_RADIUS / (1 + g * constants::REAL_RADIUS / 2);
}

qreal deltaY(qreal r, qreal eq_radius) { return (r * r) / (2 * eq_radius); }

/*customPlot->addGraph();                                   // для теста
customPlot->graph(3)->setData(it_x_vector, it_y_vector);  //

std::for_each(it_y_vector.begin(), it_y_vector.end(), [&](qreal &x) {  //
  static qint32 i = 0;                                                 //
  x += 2 * HNull[i];                                                   //
  i++;                                                                 //
});                                                                    //
customPlot->graph(4)->setData(it_x_vector, it_y_vector);               //
*/
