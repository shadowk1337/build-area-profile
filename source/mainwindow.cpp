#include "mainwindow.h"
#include <qcustomplot.h>
#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QRegExp>
#include <QTextStream>
#include <bits\stdc++.h>
#include "ui_mainwindow.h"

const double REAL_RADIUS = 6.37e+06;  // действительный радиус Земли (в метрах)
const double G_STANDARD = -8e-08;  // вертикальный градиент индекса преломления
                                   // в приземной части тропосферы
const double R_EVALUATED =
    16.9e+03;  // предельное удаление края дуги от центра (в метрах)
const double LAMBDA = 0.2;  // длина волны

class DataSaver {
 public:
  DataSaver(qint32 names_count_ = 0) : names_count(names_count_) {
    y_from_x = QMap<double, double>();
    heights = QVector<double>();
  }
  QMap<double, double> &GetMap() { return y_from_x; }
  void SetMap(const QMap<double, double> &m) {
    y_from_x = QMap<double, double>(m);
  }
  qint32 GetCount() { return names_count; }
  void SetCount(qint32 names_count_) { names_count = names_count_; }
  QVector<double> &GetHeightsArr() { return heights; }
  void SetHeightsArr(const QVector<double> &v) { heights = QVector<double>(v); }

 private:
  QMap<double, double> y_from_x;
  qint32 names_count;
  QVector<double> heights;
};

DataSaver *setupArc(QCustomPlot *arc, DataSaver *ds);
DataSaver *setupCurve(QCustomPlot *curvPlot, DataSaver *ds);
void setupAxis(QCustomPlot *axis);
DataSaver *setupFile(QCustomPlot *f);
void IntervalType(QCustomPlot *customPlot, DataSaver *ds);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  setWindowTitle("Area profile");
  DataSaver *ds_main = setupFile(ui->customPlot);
  setupArc(ui->customPlot, ds_main);
  setupCurve(ui->customPlot, ds_main);
  setupAxis(ui->customPlot);
  IntervalType(ui->customPlot, ds_main);

  textItem = new QCPItemText(ui->customPlot);
  connect(ui->customPlot, &QCustomPlot::mouseMove, this,
          &MainWindow::onMouseMove);
  ui->customPlot->replot();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::onMouseMove(QMouseEvent *event) {
  QCustomPlot *customPlot = qobject_cast<QCustomPlot *>(sender());
  double x = customPlot->xAxis->pixelToCoord(event->pos().x());
  double y = customPlot->yAxis->pixelToCoord(event->pos().y());
  textItem->setText(QString("(%1, %2)")
                        .arg(static_cast<int>(x) / 1000)
                        .arg(static_cast<int>(y)));
  textItem->position->setCoords(QPointF(x, y));
  customPlot->replot();
}

// чтение данных из файла
DataSaver *setupFile(QCustomPlot *f) {
  QVector<double> heights;
  DataSaver *ds = new DataSaver;
  QFile file("heights.csv");
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Could't open the data file\n";
    exit(EXIT_FAILURE);
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
  ds->SetCount(count);
  ds->SetHeightsArr(heights);
  return ds;
}

// настройки осей
void setupAxis(QCustomPlot *axis) {
  axis->rescaleAxes();

  axis->yAxis->scaleRange(2);
  axis->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |
                        QCP::iSelectPlottables);

  QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
  QString str;
  for (double i = 0; i < 2 * R_EVALUATED + 500; i += 500) {
    str = QString::number(static_cast<int>(i) / 1000);
    textTicker->addTick(i, str);
  }
  QFont pfont("Arial", 8);
  axis->xAxis->setTicker(textTicker);
  axis->xAxis->setTickLabelFont(pfont);
  axis->xAxis->setTickLabelRotation(-45);
  axis->xAxis->setTickLength(0);
  axis->yAxis->setSubTickLength(0);
  axis->xAxis->setRange(0, 2 * R_EVALUATED);
}

// кривая земной поверхности
double EquivalentRadius(double g);
double DeltaY(double r, double eq_radius);

DataSaver *setupArc(QCustomPlot *arcPlot, DataSaver *ds) {
  QVector<double> x(2 * R_EVALUATED), y(2 * R_EVALUATED);
  QMap<double, double> m;
  bool first = true;
  double move_graph_up_value;
  double equivalent_radius = EquivalentRadius(G_STANDARD);
  double iteration_difference = 2 * R_EVALUATED / ds->GetCount();
  for (double i = -R_EVALUATED, iterator = 0; i < R_EVALUATED;
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
  ds->SetMap(m);
  arcPlot->addGraph();
  arcPlot->graph(0)->setData(x, y);
  arcPlot->graph(0)->setPen(QPen(QColor("#014506")));
  QCPItemTracer *tracer = new QCPItemTracer(arcPlot);
  tracer->setGraph(arcPlot->graph(0));
  tracer->updatePosition();
  assert(x.size() == y.size());
  return ds;
}

void StraightViewLine(QCustomPlot *linePlot, DataSaver *ds);

// кривая высот
DataSaver *setupCurve(QCustomPlot *curvPlot, DataSaver *ds) {
  QVector<double> heights = ds->GetHeightsArr();
  qint32 count = ds->GetCount();
  QMap<double, double> y_from_x = ds->GetMap();
  curvPlot->addGraph();
  double iteration_difference = 2 * R_EVALUATED / count;

  for (double i = 0, v = 0; v < ds->GetCount(); i += iteration_difference, v++)
    heights[v] += y_from_x[i];
  QVector<double> x(ds->GetCount() - 1), y(ds->GetCount() - 1);
  double i;
  int v;
  for (i = 0, v = 0; v + 1 < ds->GetCount(); i += iteration_difference, v++) {
    x[v] = i;
    y[v] = heights[v];
  }
  assert(x.size() == y.size());
  curvPlot->addGraph();
  curvPlot->graph(1)->setData(x, y);
  curvPlot->graph(1)->setPen(QPen(QColor("#137ea8")));
  StraightViewLine(curvPlot, ds);
  ds->SetHeightsArr(heights);  // H + h map
  return ds;
}

double EquivalentRadius(double g) {
  return REAL_RADIUS / (1 + g * REAL_RADIUS / 2);
}

double DeltaY(double r, double eq_radius) { return (r * r) / (2 * eq_radius); }

// линия прямой видимости
void StraightViewLine(QCustomPlot *linePlot, DataSaver *ds) {
  QVector<double> x(ds->GetCount()), y(ds->GetCount());
  double x_start = 0;
  double y_start = 117.49;
  double x_end = 33700;
  double y_end = 52.7;
  double x_diff = (x_end - x_start) / ds->GetCount();
  double y_diff = (y_end - y_start) / ds->GetCount();
  for (int i = 0; i < ds->GetCount(); i++) {
    x[i] = x_start + i * x_diff;
    y[i] = y_start + i * y_diff;
  }
  linePlot->addGraph();
  linePlot->graph(2)->setData(x, y);
  linePlot->graph(2)->setPen(QPen(QColor("#d6ba06")));
}

void OpenedIntervalApproximation(QCustomPlot *customPlot,
                                 const QVector<qint32> &interval_type,
                                 DataSaver *ds);

#define k(R) R / (2 * R_EVALUATED)

void IntervalType(QCustomPlot *customPlot, DataSaver *ds) {
  QVector<double> line_of_sight_heights(customPlot->graph(2)->dataCount());
  QVector<double> curve_heights = ds->GetHeightsArr();
  QVector<double> H_from_k(customPlot->graph(2)->dataCount());
  QVector<double> H0_from_k(customPlot->graph(2)->dataCount());
  QVector<double> h0(customPlot->graph(2)->dataCount());
  QVector<qint32> interval_type(
      customPlot->graph(2)
          ->dataCount());  // 1 - Открытый, 2 - Полуоткрытый, 3 - Закрытый
  int v_size = customPlot->graph(2)->dataCount();
  double intervals_diff = 2 * R_EVALUATED / customPlot->graph(2)->dataCount();
  for (int i = 0; i < v_size; ++i)
    line_of_sight_heights[i] = customPlot->graph(2)
                                   ->dataValueRange(i * (intervals_diff / 100))
                                   .center();
  for (int i = 0; i < v_size; ++i)
    H_from_k[i] = line_of_sight_heights[i] - curve_heights[i];
  double z = 0;
  for (int i = 0; i < v_size; ++i, z += intervals_diff)
    H0_from_k[i] = sqrt(z * LAMBDA * k(z) * (1 - k(z)) / 3);
  for (int i = 0; i < v_size; ++i) h0[i] = H_from_k[i] / H0_from_k[i];
  for (int i = 0; i < v_size; ++i) {
    if (H_from_k[i] > H0_from_k[i] && h0[i] >= 0)
      interval_type[i] = 1;
    else if (H0_from_k[i] > H_from_k[i] && H_from_k[i] > 0 && h0[i] > 0)
      interval_type[i] = 2;
    else if (H0_from_k[i] > H_from_k[i] && h0[i] < 0)
      interval_type[i] = 3;
    else {
      qDebug() << "Interval " << 2 * R_EVALUATED / i << " wasn't defined";
      exit(EXIT_FAILURE);
    }
  }
  OpenedIntervalApproximation(customPlot, interval_type, ds);
}

/*void AB_type_lines(QVector<QPair<QPointF, QPointF>> &v, const QPointF &start,
const QPointF &end){ v.push_back(QPair<QPointF, QPointF>(start, end));
}*/

void GD_type_lines(QVector<QPair<QPointF, QPointF>> &v_to_push,
                   QVector<double>::const_iterator begin,
                   QVector<double>::const_iterator end,
                   QVector<double>::const_iterator graph_start,
                   double iterator_multiply) {
  QVector<double>::const_iterator max_height_first_half =
      std::max_element(begin, begin + std::distance(begin, end) / 2);
  QVector<double>::const_iterator max_height_last_half =
      std::max_element(begin + std::distance(begin, end) / 2, end);
  std::cout << *max_height_first_half << " " << *max_height_last_half << '\n';
  v_to_push.push_back(QPair<QPointF, QPointF>(
      QPointF(std::distance(graph_start, max_height_first_half) *
                  iterator_multiply,  // нужно, чтобы было число
              *max_height_first_half),
      QPointF(
          std::distance(graph_start, max_height_last_half) * iterator_multiply,
          *max_height_last_half)));
}

void FindIntersectionXCoord(
    const QCustomPlot *customPlot,
    const QVector<QPair<QPointF, QPointF>> &making_lines_points,
    const QVector<QPair<QPointF, QPointF>> &land_lines_points);

void OpenedIntervalApproximation(QCustomPlot *customPlot,
                                 const QVector<qint32> &interval_type,
                                 DataSaver *ds) {
  QVector<double> x, y;
  QVector<double> land_heights = ds->GetHeightsArr();
  QVector<QPair<QPointF, QPointF>>
      AB_lines_border_point,  // крайние точки линий А'Б
      GD_lines_border_point;  // крайние точки линий ГД
  double intervals_diff = 2 * R_EVALUATED / ds->GetCount();
  qint32 it_line_start, it_line_end;
  QCPGraph *graph;
  QCPItemLine *line;
  bool end = true;
  for (qint32 i = 0; i < interval_type.size(); ++i) {
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
        double y_axis_diff =
            (land_heights[it_line_end] - land_heights[it_line_start]) /
            (it_line_end - it_line_start);
        for (int t = it_line_start, y_mult = 0; t <= it_line_end;
             ++t, ++y_mult) {
          x.push_back(t * intervals_diff);
          y.push_back(land_heights[it_line_start] + 25 + y_mult * y_axis_diff);
        }
        assert(x.size() == y.size());
        line = new QCPItemLine(customPlot);  // прямые А'Б
        line->start->setCoords(*x.begin(),
                               land_heights[it_line_start] -
                                   (*y.begin() - land_heights[it_line_start]));
        line->end->setCoords(*(x.end() - 1), *(y.end() - 1));
        graph = new QCPGraph(customPlot->xAxis, customPlot->yAxis);
        graph->setData(x, y);
        AB_lines_border_point.push_back(QPair<QPointF, QPointF>(
            QPointF(*x.begin(), land_heights[it_line_start] -
                                    (*y.begin() - land_heights[it_line_start])),
            QPointF(*(x.end() - 1), *(y.end() - 1))));
        GD_type_lines(GD_lines_border_point,
                      land_heights.begin() + it_line_start,
                      land_heights.begin() + it_line_end, land_heights.begin(),
                      intervals_diff);
      }
      end = true;
    }
  }
  for (const auto &item : GD_lines_border_point) {
    line = new QCPItemLine(customPlot);  // прямые ГД
    line->setPen(QPen(QColor(Qt::red)));
    line->start->setCoords(item.first.x(), item.first.y());
    line->end->setCoords(item.second.x(), item.second.y());
  }
  FindIntersectionXCoord(customPlot, AB_lines_border_point,
                         GD_lines_border_point);
}

// qint32 LengthOfReflection() {}

void FindIntersectionXCoord(
    const QCustomPlot *customPlot,
    const QVector<QPair<QPointF, QPointF>> &making_lines_points,
    const QVector<QPair<QPointF, QPointF>> &land_lines_points) {
  assert(making_lines_points.size() == land_lines_points.size());
  for (const auto &item : making_lines_points, land_lines_points) {
    double der = (double)(item.second.y() - item.first.y()) /
                 (item.second.x() - item.first.x());
    qDebug() << item.first.y() - item.first.x() * der << " + x" << der;
  }
  /*for (const auto &item : land_lines_points) {
    double der = (double)(item.second.y() - item.first.y()) /
                 (item.second.x() - item.first.x());
    qDebug() << item.first.y() - item.first.x() * der << " + x" << der;
  }*/
}
