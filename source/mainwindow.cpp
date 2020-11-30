#include "mainwindow.h"
#include <qcustomplot.h>
#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QRegExp>
#include <QTextStream>
#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <new>
#include "ui_mainwindow.h"

const qreal REAL_RADIUS = 6.37e+06;  // действительный радиус Земли (в метрах)
const qreal G_STANDARD = -8e-08;  // вертикальный градиент индекса преломления
                                  // в приземной части тропосферы
const qreal R_EVALUATED =
    16.9e+03;  // предельное удаление края дуги от центра (в метрах)
const qreal LAMBDA = 0.2;  // длина волны

class {
 public:
  QMap<qreal, qreal> &GetMap(void) { return y_from_x; }
  void SetMap(const QMap<qreal, qreal> &m) { y_from_x = QMap<qreal, qreal>(m); }
  qint32 GetCount(void) { return names_count; }
  void SetCount(qint32 names_count_) { names_count = names_count_; }
  QVector<qreal> &GetHeightsArr(void) { return heights; }
  void SetHeightsArr(const QVector<qreal> &v) { heights = QVector<qreal>(v); }

 private:
  QMap<qreal, qreal> y_from_x;
  qint32 names_count;
  QVector<qreal> heights;
} useful_data;

template <typename T>
class VectorSaver {
 public:
  void SetVector(const QVector<T> &v_) { v = v_; }
  QVector<T> GetVector(void) { return v; }

 private:
  QVector<T> v;
};

VectorSaver<qreal> saver;

bool SetupFile(void);
void SetupArc(QCustomPlot *arc);
void SetupAxis(QCustomPlot *axis);
void SetupCurve(QCustomPlot *curvPlot);
void IntervalType(QCustomPlot *customPlot);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  setWindowTitle("Area profile");
  assert(SetupFile() == true);
  SetupArc(ui->customPlot);
  SetupAxis(ui->customPlot);
  SetupCurve(ui->customPlot);
  IntervalType(ui->customPlot);

  textItem = new QCPItemText(ui->customPlot);
  connect(ui->customPlot, &QCustomPlot::mouseMove, this,
          &MainWindow::onMouseMove);
  ui->customPlot->replot();
}

MainWindow::~MainWindow() { delete ui; }

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
bool SetupFile(void) {
  QVector<qreal> heights;
  QFile file("heights.csv");
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Could't open the data file\n";
    return false;
  }
  QTextStream in(&file);
  QString test;
  QRegExp rx("[ ;]");
  qint32 count = 0;
  bool first = true;
  while (!in.atEnd()) {
    QString line = in.readLine();
    if (first) {
      first = false;
      continue;
    }
    line = line.section(";", 3, 3);
    line.replace(",", ".");
    heights.push_back(line.toDouble());
    count++;
  }
  assert(heights.size() <= count);
  file.close();
  if (!count) qDebug() << "File is empty";
  useful_data.SetCount(count);
  useful_data.SetHeightsArr(heights);
  return true;
}

// Настройки осей
void SetupAxis(QCustomPlot *axis) {
  axis->rescaleAxes();

  axis->yAxis->scaleRange(2);
  axis->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |
                        QCP::iSelectPlottables);

  /*QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
  QString str;
  for (qreal i = 0; i < 2 * R_EVALUATED + 500; i += 500) {
    str = QString::number(static_cast<qint32>(i) / 1000);
    textTicker->addTick(i, str);
  }
  QFont pfont("Arial", 8);
  axis->xAxis->setTicker(textTicker);
  axis->xAxis->setTickLabelFont(pfont);
  axis->xAxis->setTickLabelRotation(-45);
  axis->xAxis->setTickLength(0);
  axis->yAxis->setSubTickLength(0);
  axis->xAxis->setRange(0, 2 * R_EVALUATED);*/
}

// кривая земной поверхности
qreal EquivalentRadius(qreal g);
qreal DeltaY(qreal r, qreal eq_radius);

void SetupArc(QCustomPlot *arcPlot) {
  QVector<qreal> x(2 * R_EVALUATED), y(2 * R_EVALUATED);
  QMap<qreal, qreal> m;
  bool first = true;
  qreal move_graph_up_value;
  qreal equivalent_radius = EquivalentRadius(G_STANDARD);
  qreal iteration_difference = 2 * R_EVALUATED / useful_data.GetCount();
  for (qreal i = -R_EVALUATED, iterator = 0; i < R_EVALUATED;
       i += iteration_difference, iterator += iteration_difference) {
    x[i + R_EVALUATED] = i + R_EVALUATED;
    if (first) {
      move_graph_up_value = abs(DeltaY(i, equivalent_radius));
      y[i + R_EVALUATED] = 0;
      first = false;
      m[iterator] = y[i + R_EVALUATED];
    } else {
      y[i + R_EVALUATED] = -DeltaY(i, equivalent_radius) + move_graph_up_value;
      m[iterator] = y[i + R_EVALUATED];
    }
  }
  useful_data.SetMap(m);
  arcPlot->addGraph();
  arcPlot->graph(0)->setData(x, y);
  arcPlot->graph(0)->setPen(QPen(QColor("#014506")));
  QCPItemTracer *tracer = new QCPItemTracer(arcPlot);
  tracer->setGraph(arcPlot->graph(0));
  tracer->updatePosition();
  assert(x.size() == y.size());
}

void LineOfSight(QCustomPlot *linePlot);

// Кривая высот
void SetupCurve(QCustomPlot *curvPlot) {
  QVector<qreal> heights = useful_data.GetHeightsArr();
  curvPlot->addGraph();
  qreal iteration_difference = 2 * R_EVALUATED / useful_data.GetCount();
  for (qreal i = 0, v = 0; v < useful_data.GetCount();
       i += iteration_difference, v++)
    heights[v] += useful_data.GetMap()[i];
  QVector<qreal> x(useful_data.GetCount() - 1), y(useful_data.GetCount() - 1);
  qreal i;
  int v;
  for (i = 0, v = 0; v + 1 < useful_data.GetCount();
       i += iteration_difference, v++) {
    x[v] = i;
    y[v] = heights[v];
  }
  assert(x.size() == y.size());
  curvPlot->addGraph();
  curvPlot->graph(1)->setData(x, y);
  curvPlot->graph(1)->setPen(QPen(QColor("#137ea8")));
  LineOfSight(curvPlot);
  useful_data.SetHeightsArr(heights);  // H + h map
}

// Линия прямой видимости
void LineOfSight(QCustomPlot *linePlot) {
  QVector<qreal> x(useful_data.GetCount()), y(useful_data.GetCount());
  qreal x_start = 0;
  qreal y_start = 117.49;
  qreal x_end = 33700;
  qreal y_end = 52.7;
  qreal x_diff = (x_end - x_start) / useful_data.GetCount();
  qreal y_diff = (y_end - y_start) / useful_data.GetCount();
  for (qint32 i = 0; i < useful_data.GetCount(); i++) {
    x[i] = x_start + i * x_diff;
    y[i] = y_start + i * y_diff;
  }
  linePlot->addGraph();
  linePlot->graph(2)->setData(x, y);
  linePlot->graph(2)->setPen(QPen(QColor("#d6ba06")));
}

void OpenedIntervalApproximation(QCustomPlot *customPlot,
                                 const QVector<qint32> &interval_type);

void IntervalTypeCalc(QCustomPlot *customPlot,
                      const QVector<qint32> &interval_type);

inline qreal k_from_R(qreal R) { return R / (2 * R_EVALUATED); }

// Определение типов интервалов
void IntervalType(QCustomPlot *customPlot) {
  QVector<qreal> curve_heights = useful_data.GetHeightsArr();
  QVector<qreal> line_of_sight_heights(customPlot->graph(2)->dataCount()),
      H_from_k(customPlot->graph(2)->dataCount()),
      H0_from_k(customPlot->graph(2)->dataCount()),
      h0(customPlot->graph(2)->dataCount());
  QVector<qint32> interval_type(
      customPlot->graph(2)
          ->dataCount());  // 1 - Открытый, 2 - Полуоткрытый, 3 - Закрытый
  qint32 v_size = customPlot->graph(2)->dataCount();
  qreal intervals_diff = 2 * R_EVALUATED / customPlot->graph(2)->dataCount();
  for (qint32 i = 0; i < v_size; ++i)
    line_of_sight_heights[i] = customPlot->graph(2)
                                   ->dataValueRange(i * (intervals_diff / 100))
                                   .center();
  for (qint32 i = 0; i < v_size; ++i)
    H_from_k[i] = line_of_sight_heights[i] - curve_heights[i];
  qreal z = 0;
  for (qint32 i = 0; i < v_size; ++i, z += intervals_diff)
    H0_from_k[i] =
        sqrt(2 * R_EVALUATED * LAMBDA * k_from_R(z) * (1 - k_from_R(z)) / 3);
  for (qint32 i = 0; i < v_size; ++i) {
    h0[i] = H_from_k[i] / H0_from_k[i];
    if (H_from_k[i] >= H0_from_k[i] && h0[i] >= 0)
      interval_type[i] = 1;
    else if (H0_from_k[i] > H_from_k[i] && H_from_k[i] > 0.1 && h0[i] < 0.1 &&
             h0[i] > 0)
      interval_type[i] = 2;
    else if (H0_from_k[i] > H_from_k[i] && h0[i] < 0)
      interval_type[i] = 3;
    else {
      interval_type[i] = 1;
    }
    //    to_save.push_back(H0_from_k[i] / h0[i]);
  }
  assert(H0_from_k.size() == h0.size());
  QVector<qreal> to_save;
  for (auto i = 0; i < h0.size(); ++i) {
    to_save.push_back(H0_from_k[i] / h0[i]);
  }
  saver.SetVector(to_save);  //  вектор со значениями H0 / h0 ---> save вектор
  IntervalTypeCalc(customPlot, interval_type);
}

void AB_type_lines(QCustomPlot *customPlot, const QVector<qreal> &x,
                   const QVector<qreal> &y, qint32 it_line_start,
                   QVector<QPair<QPointF, QPointF>> &AB_lines_border_point) {
  QCPItemLine *line = new QCPItemLine(customPlot);
  line->start->setCoords(
      *x.begin(),
      useful_data.GetHeightsArr()[it_line_start] -
          (*y.begin() - useful_data.GetHeightsArr()[it_line_start]));
  line->end->setCoords(*(x.end() - 1), *(y.end() - 1));
  AB_lines_border_point.push_back(QPair<QPointF, QPointF>(
      QPointF(*x.begin(),
              useful_data.GetHeightsArr()[it_line_start] -
                  (*y.begin() - useful_data.GetHeightsArr()[it_line_start])),
      QPointF(*(x.end() - 1), *(y.end() - 1))));
}

// Линни ГД типа для открытых интервалов
void GD_type_lines(QCustomPlot *customPlot,
                   QVector<QPair<QPointF, QPointF>> &v_to_push,
                   QVector<qreal>::const_iterator begin,
                   QVector<qreal>::const_iterator end,
                   QVector<qreal>::const_iterator graph_start,
                   qreal iterator_multiply) {
  QVector<qreal>::const_iterator max_height_first_half =
      std::max_element(begin, begin + std::distance(begin, end) / 2);
  QVector<qreal>::const_iterator max_height_last_half =
      std::max_element(begin + std::distance(begin, end) / 2, end);
  v_to_push.push_back(QPair<QPointF, QPointF>(
      QPointF(
          std::distance(graph_start, max_height_first_half) * iterator_multiply,
          *max_height_first_half),
      QPointF(
          std::distance(graph_start, max_height_last_half) * iterator_multiply,
          *max_height_last_half)));
  QCPItemLine *line = new QCPItemLine(customPlot);
  for (const auto &item : v_to_push) {
    line = new QCPItemLine(customPlot);  // прямые ГД
    line->setPen(QPen(QColor(Qt::red)));
    line->start->setCoords(item.first.x(), item.first.y());
    line->end->setCoords(item.second.x(), item.second.y());
  }
}

QVector<qreal> FindIntersectionXCoord(
    const QVector<QPair<QPointF, QPointF>> &making_lines_points,
    const QVector<QPair<QPointF, QPointF>> &land_lines_points);

void IntervalTypeCalc(QCustomPlot *customPlot,
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
  if (!first_type_indexes.empty())
    OpenedIntervalApproximation(customPlot, first_type_indexes);
  /*else if (!second_type_indexes.empty())
    HalfOpenedIntervalApproximation(customPlot, interval_type);
  else if (!third_type_indexes.empty())
    ClosedIntervalApproximation(customPlot, interval_type);*/
}

inline qreal l0(qreal h0, qreal k) {
  return ((2 * R_EVALUATED) * sqrt(1 + h0 * h0)) /
         (1 + h0 * h0 / (4 * k * (1 - k)));
}

// Критерий Рэлея
void RayleighCriteria(qint32 index_line_start, qint32 index_line_end);

// Открытые интервалы
void OpenedIntervalApproximation(QCustomPlot *customPlot,
                                 const QVector<qint32> &interval_type) {
  QVector<qreal> x, y;
  QVector<qreal> land_heights = useful_data.GetHeightsArr();
  qreal intervals_diff = 2 * R_EVALUATED / useful_data.GetCount();
  QVector<QPair<QPointF, QPointF>>
      AB_lines_border_point,  // крайние точки линий А'Б
      GD_lines_border_point;  // крайние точки линий ГД
  qint32 idx_line_start, idx_line_end, prev;
  idx_line_start = prev = interval_type[0];
  QCPItemLine *line;
  for (auto it : interval_type) {
    if (abs(it - prev) > 1 || it == *(interval_type.end() - 1)) {
      line = new QCPItemLine(customPlot);
      idx_line_end = prev;
      line->start->setCoords(idx_line_start * intervals_diff,
                             useful_data.GetHeightsArr()[idx_line_start]);
      line->end->setCoords(idx_line_end * intervals_diff,
                           useful_data.GetHeightsArr()[idx_line_end]);
      RayleighCriteria(idx_line_start, idx_line_end);
      idx_line_start = it;
    }
    prev = it;
  }
  /*for (qint32 i = 0; i < interval_type.size(); ++i) {
    if (interval_type[i] == 1 && i != interval_type.size() - 1) {
      if (end) {
        x.clear();
        y.clear();
        it_line_start = i;
        end = false;
      } else {
        it_line_end = i;
      }
    } else {
      if (!end) {
        if (i == interval_type.size() - 1) it_line_end = i;
        qreal y_axis_diff =
            (land_heights[it_line_end] - land_heights[it_line_start]) /
            (it_line_end - it_line_start);
        for (int t = it_line_start, y_mult = 0; t <= it_line_end;
             ++t, ++y_mult) {
          x.push_back(t * intervals_diff);
          y.push_back(land_heights[it_line_start] + 25 + y_mult * y_axis_diff);
        }
        assert(x.size() == y.size());
        graph = new QCPGraph(customPlot->xAxis, customPlot->yAxis);
        graph->setData(x, y);
        AB_type_lines(customPlot, x, y, it_line_start, AB_lines_border_point);
        GD_type_lines(customPlot, GD_lines_border_point,
                      land_heights.begin() + it_line_start,
                      land_heights.begin() + it_line_end, land_heights.begin(),
                      intervals_diff);
      }
      end = true;
    }
  }*/
}

void RayleighCriteria(qint32 index_line_start, qint32 index_line_end) {
  qreal k, b, denominator, y, delta_h;
  QVector<qreal> H0_h0_division = saver.GetVector();
  denominator = (index_line_end - index_line_start);
  k = (useful_data.GetHeightsArr().at(index_line_end) -
       useful_data.GetHeightsArr().at(index_line_start)) /
      denominator;
  b = (index_line_end * useful_data.GetHeightsArr().at(index_line_start) -
       index_line_start * useful_data.GetHeightsArr().at(index_line_end)) /
      denominator;
  for (auto i = index_line_start; i < index_line_end; ++i) {
    y = k * i + b;
    delta_h = abs(y - useful_data.GetHeightsArr().at(i));
    //    qDebug() << i << " " << delta_h << " " << H0_h0_division[i];
    //    if (delta_h > 0.75 * H0_h0_division[i])
    //      qDebug() << i;
  }
}

void HalfOpenedIntervalApproximation(QCustomPlot *customPlot, const QVector<qint32> &interval_type){

}


// qint32 LengthOfReflection(QVector<qreal> &h0) {

// }

// Нахождение точки пересечения двух линий, заданных двумя точками
QVector<qreal> FindIntersectionXCoord(
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
  for (auto x : x_coords)
    qDebug() << x;
  for (const auto &item : making_lines_points) {
    qreal der = (qreal)(item.second.y() - item.first.y()) /
                 (item.second.x() - item.first.x());
    qDebug() << item.first.y() - item.first.x() * der << " + x" << der;
  }
  for (const auto &item : land_lines_points) {
    qreal der = (qreal)(item.second.y() - item.first.y()) /
                 (item.second.x() - item.first.x());
    qDebug() << item.first.y() - item.first.x() * der << " + x" << der;
  }
}

qreal EquivalentRadius(qreal g) {
  return REAL_RADIUS / (1 + g * REAL_RADIUS / 2);
}

qreal DeltaY(qreal r, qreal eq_radius) { return (r * r) / (2 * eq_radius); }
