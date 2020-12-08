#include "mainwindow.h"
#include <qcustomplot.h>
#include <usefuldata.h>
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
#include <new>
#include "constants.h"
#include "ui_mainwindow.h"

struct {
  qint32 s_counter;
  qreal s_intervals_difference;
  UsefulData::Map<qreal, qreal> s_map;
  UsefulData::Vector<qreal> s_heights, s_HNull_hNull_div, s_hNull;
} useful_data;

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
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Could't open the data file";
    return false;
  }
  QVector<qreal> heights;
  QTextStream in(&file);
  QRegExp rx("[ ;]");
  bool first = true;
  while (!in.atEnd()) {
    QString line = in.readLine();
    if (first) {
      first = false;
      continue;
    }
    line.replace(",", ".");
    heights.push_back(line.section(";", 3, 3).toDouble());
    useful_data.s_counter++;
  }
  assert(heights.size() <= useful_data.s_counter);
  file.close();

  if (!useful_data.s_counter) {
    qDebug() << "File is empty";
    return false;
  }
  useful_data.s_heights.setVector(heights);
  useful_data.s_intervals_difference =
      constants::AREA_LENGTH / useful_data.s_counter;
  return true;
}

// Настройки осей
void MainWindow::setupAxis(QCustomPlot *axis) {
  axis->rescaleAxes();

  axis->yAxis->scaleRange(2);
  axis->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |
                        QCP::iSelectPlottables);

  QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
  QString str;
  for (qreal i = 0; i < constants::AREA_LENGTH + 500; i += 500) {
    str = QString::number(static_cast<qint32>(i) / 1000);
    textTicker->addTick(i, str);
  }
  QFont pfont("Arial", 8);
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
  QMap<qreal, qreal> m;
  bool first = true;
  qreal move_graph_up_value;
  qreal equivalent_radius = equivalentRadius(constants::G_STANDARD);
  qreal s_intervals_difference = constants::AREA_LENGTH / useful_data.s_counter;
  for (qreal i = -constants::R_EVALUATED, iterator = 0;
       i < constants::R_EVALUATED;
       i += s_intervals_difference, iterator += s_intervals_difference) {
    x[i + constants::R_EVALUATED] = i + constants::R_EVALUATED;
    if (first) {
      move_graph_up_value = qAbs(deltaY(i, equivalent_radius));
      y[i + constants::R_EVALUATED] = 0;
      first = false;
      m[iterator] = y[i + constants::R_EVALUATED];
    } else {
      y[i + constants::R_EVALUATED] =
          -deltaY(i, equivalent_radius) + move_graph_up_value;
      m[iterator] = y[i + constants::R_EVALUATED];
    }
  }
  useful_data.s_map.setMap(m);
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
  QVector<qreal> heights = useful_data.s_heights.getVector();
  for (qreal i = 0, v = 0; v < useful_data.s_counter;
       i += useful_data.s_intervals_difference, v++)
    heights[v] += useful_data.s_map[i];
  QVector<qreal> x(useful_data.s_counter - 1), y(useful_data.s_counter - 1);
  qreal i;
  qint32 v;
  for (i = 0, v = 0; v + 1 < useful_data.s_counter;
       i += useful_data.s_intervals_difference, v++) {
    x[v] = i;
    y[v] = heights[v];
  }
  assert(x.size() == y.size());
  curvPlot->addGraph();
  curvPlot->graph(1)->setData(x, y);
  curvPlot->graph(1)->setPen(QPen(QColor("#137ea8")));
  lineOfSight(curvPlot);
  useful_data.s_heights.setVector(heights);  // H + h map
}

// Линия прямой видимости
void lineOfSight(QCustomPlot *linePlot) {
  QVector<qreal> x(useful_data.s_counter), y(useful_data.s_counter);
    qreal x_start = 0;
    qreal y_start = 117.49;
    qreal x_end = 33700;
    qreal y_end = 52.7;
    qreal x_diff = (x_end - x_start) / useful_data.s_counter;
    qreal y_diff = (y_end - y_start) / useful_data.s_counter;
    for (qint32 i = 0; i < useful_data.s_counter; i++) {
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

// Определение типов интервалов // TODO
void MainWindow::intervalType(QCustomPlot *customPlot) {
  customPlot->addGraph();
  QVector<qreal> curve_heights = useful_data.s_heights.getVector();
  QVector<qreal> line_of_sight_heights(useful_data.s_counter),
      H(useful_data.s_counter), HNull(useful_data.s_counter),
      hNull(useful_data.s_counter), it_x_vector, it_y_vector;
  QVector<qint32> interval_type(
      useful_data.s_counter);  // 1 - Открытый, 2 - Полуоткрытый, 3 - Закрытый
  for (qint32 i = 0; i < useful_data.s_counter; ++i) {
    line_of_sight_heights[i] =
        customPlot->graph(2)
            ->dataValueRange(i * (useful_data.s_intervals_difference / 100))
            .center();
    H[i] = line_of_sight_heights[i] - curve_heights[i];
    HNull[i] =
        sqrt(constants::AREA_LENGTH * constants::LAMBDA *
             k_from_R(i * useful_data.s_intervals_difference) *
             ((1 - k_from_R(i * useful_data.s_intervals_difference)) / 3));
    hNull[i] = H[i] / HNull[i];
    it_x_vector.push_back(i * useful_data.s_intervals_difference);
    it_y_vector.push_back(-HNull.at(i) + line_of_sight_heights[i]);
    typeDefinition(H, HNull, hNull, interval_type);
  }
  assert(HNull.size() == hNull.size());
  customPlot->addGraph();                                   // для теста
  customPlot->graph(3)->setData(it_x_vector, it_y_vector);  //

  std::for_each(it_y_vector.begin(), it_y_vector.end(), [&](qreal &x) {  //
    static qint32 i = 0;                                                 //
    x += 2 * HNull[i];                                                   //
    i++;                                                                 //
  });                                                                    //
  customPlot->graph(4)->setData(it_x_vector, it_y_vector);               //
  QVector<qreal> to_save;
  for (auto i = 0; i < hNull.size(); ++i) {
    to_save.push_back(HNull[i] / hNull[i]);
  }
  useful_data.s_HNull_hNull_div.setVector(
      to_save);  //  вектор со значениями hNull / hNull ---> save вектор
  useful_data.s_hNull.setVector(hNull);
  intervalTypeCalc(customPlot, interval_type);
}

void openedInterval(const QVector<qint32> &);
void halfopenedInterval(const QVector<qint32> &);

QVector<qreal> findIntersectionXCoord(const QVector<QPair<QPointF, QPointF>> &,
                                      const QVector<QPair<QPointF, QPointF>> &);

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
  //  if (!third_type_indexes.empty())
  //    ClosedIntervalApproximation(customPlot, interval_type);
}

inline qreal l0(qreal h0, qreal k) {
  return ((constants::AREA_LENGTH)*qSqrt(1 + h0 * h0)) /
         (1 + (h0 * h0) / (4 * k * (1 - k)));
}

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
        if ((l0_length = l0(useful_data.s_hNull[i],
                            i * useful_data.s_intervals_difference /
                                (constants::AREA_LENGTH))) <=
            0.25 * constants::AREA_LENGTH) {
          openedIntervalPlaneApproximation(
              std::max(
                  static_cast<qint32>(
                      i - l0_length / (2 * useful_data.s_intervals_difference)),
                  0),
              std::min(
                  static_cast<qint32>(
                      i + l0_length / (2 * useful_data.s_intervals_difference)),
                  static_cast<qint32>(constants::AREA_LENGTH)),
              i);
        } else {
          openedIntervalSphereApproximation(
              std::max(
                  static_cast<qint32>(
                      i - l0_length / (2 * useful_data.s_intervals_difference)),
                  0),
              std::min(
                  static_cast<qint32>(
                      i + l0_length / (2 * useful_data.s_intervals_difference)),
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
  QVector<qreal> H0_h0_div = useful_data.s_HNull_hNull_div.getVector();
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
  auto heights = useful_data.s_heights.getVector();
  denominator = (line_end - line_start);
  k = (heights.at(line_end) -  // TODO
       heights.at(line_start) / denominator);
  b = (line_end * heights.at(line_start) - line_start * heights.at(line_end)) /
      denominator;
  auto max_H0_h0 =
      0.75 * useful_data.s_HNull_hNull_div.getVector().at(idx_line_mid);
  for (auto i = line_start; i <= line_end; ++i) {
    y = k * i + b;
    delta_h = abs(y - heights.at(i));
    //    if (delta_h < 0.75 * max_H0_h0) qDebug() << i << ": Гладкая";
  }
}

inline qreal obstacleSphereRadius(qreal l0, qreal delta_y) {
  return (l0 * l0) / (8 * delta_y);
}

inline qreal areaReliefParameter(qreal R, qreal k, qreal H0,
                                 qreal obstacleShereRadius) {
  return pow(R * R * k * k * ((1 - k) * (1 - k)) / (obstacleShereRadius * H0),
             1.0 / 3);
}

inline qreal attenuationP(qreal areaReliefParameter) {
  return 6 + 16.4 / (areaReliefParameter * (1 + 0.8 * areaReliefParameter));
}

void halfopenedInterval(const QVector<qint32> &interval_type) {
  qint32 idx_interval_start, idx_interval_end, prev, l0, delta_y;
  idx_interval_start = prev = interval_type[0];
  qreal min_height, max_height;
  QVector<qreal> land_heights = useful_data.s_heights.getVector();
  for (auto it : interval_type) {
    if (it - prev > 3 ||
        it == *(interval_type.end() -
                1)) {  // допускаем наличие интервалов другого типа между
                       // интервалами полуоткрытого типа
      idx_interval_end = prev;
      auto [min, max] =
          std::minmax_element(land_heights.begin() + idx_interval_start,
                              land_heights.begin() + idx_interval_end);
      idx_interval_start = it;
    }
    prev = it;
  }
}

// qint32 LengthOfReflection(QVector<qreal> &h0) {

// }

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
  for (const auto &item : making_lines_points) {
    qreal der = (qreal)(item.second.y() - item.first.y()) /
                (item.second.x() - item.first.x());
  }
  for (const auto &item : land_lines_points) {
    qreal der = (qreal)(item.second.y() - item.first.y()) /
                (item.second.x() - item.first.x());
  }
}

qreal equivalentRadius(qreal g) {
  return constants::REAL_RADIUS / (1 + g * constants::REAL_RADIUS / 2);
}

qreal deltaY(qreal r, qreal eq_radius) { return (r * r) / (2 * eq_radius); }
