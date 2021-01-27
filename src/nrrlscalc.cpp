#include "nrrlscalc.h"

namespace NRrls {

namespace Calc {

namespace Fill {

/**
 * Составляющая расчета. Заполнение данными
 */
class Item : public Calc::Item {
 public:
  QSHDEF(Item);
  Item(Calc::Data::WeakPtr &data, QCustomPlot *cp) : Calc::Item(data, cp) {}

 public:
  bool exec() override;

 private:
  QSharedPointer<NRrls::Calc::Data> data = _data.toStrongRef();
};

}  // namespace Fill

namespace Profile {

/**
 * Составляющая расчета. Построение графика. Базовый класс
 */
class Item : public Calc::Item {
 public:
  QSHDEF(Item);
  Item(const Calc::Data::WeakPtr &data, QCustomPlot *cp)
      : Calc::Item(data, cp) {}

 public:
  virtual bool exec() override;

 protected:
  QSharedPointer<NRrls::Calc::Data> data = _data.toStrongRef();
};

/**
 * Составляющая расчета. Задание осей графика
 */
class Axes : public Item {
 public:
  QSHDEF(Axes);
  Axes(const Calc::Data::WeakPtr &data, QCustomPlot *cp)
      : Profile::Item(data, cp) {}

 public:
  bool exec() override;
};

/**
 * Составляющая расчета. Построение земной поверхности
 */
class Earth : public Item {
 public:
  QSHDEF(Earth);
  Earth(const Calc::Data::WeakPtr &data, QCustomPlot *cp)
      : Profile::Item(data, cp) {}

 public:
  bool exec() override;
};

/**
 * Составляющая расчета. Построение высотного профиля
 */
class Curve : public Item {
 public:
  QSHDEF(Curve);
  Curve(const Calc::Data::WeakPtr &data, QCustomPlot *cp)
      : Profile::Item(data, cp) {}

 public:
  bool exec() override;
};

/**
 * Составляющая расчета. Построение зоны Френеля
 */
class Fresnel : public Item {
 public:
  QSHDEF(Fresnel);
  Fresnel(const Calc::Data::WeakPtr &data, QCustomPlot *cp)
      : Profile::Item(data, cp) {}

 public:
  bool exec() override;
};

/**
 * Составляющая расчета. Построение ЛПВ
 */
class Los : public Item {
 public:
  QSHDEF(Los);
  Los(const Calc::Data::WeakPtr &data, QCustomPlot *cp)
      : Profile::Item(data, cp) {}

 public:
  bool exec() override;
};

}  // namespace Profile

namespace Interval {

/**
 * Составляющая расчета. Определение интервала
 */
class Item : public Calc::Item {
 public:
  QSHDEF(Item);
  Item(const Data::WeakPtr &data) : Calc::Item(data) {}

 public:
  virtual bool exec() override;

 protected:
  QSharedPointer<NRrls::Calc::Data> data = _data.toStrongRef();
};

}  // namespace Interval

namespace Atten {

namespace Land {

/**
 * Составляющая расчета. Расчет затухания в рельефе
 */
class Item : public Interval::Item {
 public:
  QSHDEF(Item);
  Item(const Data::WeakPtr &data) : Interval::Item(data) {}

 public:
  bool exec() override;

 protected:
  QSharedPointer<NRrls::Calc::Data> data = _data.toStrongRef();
  std::pair<double, double> strLineEquation(double x, double y, double x_relief,
                                            double y_relief) const;
  int int_start = 0;
  int int_end = data->param.count - 1;
};

/**
 * Составляющая расчета. Расчет затухания на открытом интервале
 */
class Opened : public Item {
 public:
  QSHDEF(Item);
  Opened(const Data::WeakPtr &data) : Item(data) {}

 public:
  bool exec() override;

 private:
  void findPointOfIntersection(void);
  void PlaneApproximation(void);
  void SphereApproximation(void);
  void rayleighAndGroundCriteria(void);
  double findEl(int piv, int limit);
  std::pair<int, int> lineOfSightCoords(void);

 private:
  double attentuationPlane(double);
};

/**
 * Составляющая расчета. Расчет затухания на полуоткрытом интервале
 */
class SemiOpened : public Item {
 public:
  QSHDEF(Item);
  SemiOpened(const Data::WeakPtr &data) : Item(data) {}

 public:
  bool exec() override;

 private:
  void approx(void);
  std::pair<double, double> SphereApproximation(int idx,
                                                double obst_sph_radius);
  double WedgeApproximation(int idx);
  double deltaY(void);
  std::pair<int, double> shadingObstacle(void) const;

 private:
  inline double areaReliefParameter(double, double, double);
  inline double reliefParFuncSph(double);
  inline double uniteApprox(double, double);
  inline double attenuationPSph(double);
  inline double nuWedg(double, double);
  inline double attentuationPWedg(double);
  inline double t(double, double);
};

/**
 * Составляющая расчета. Расчет затухания на закрытом интервале
 */
class Closed : public Item {
 public:
  QSHDEF(Item);
  Closed(const Data::WeakPtr &data) : Item(data) {}

 public:
  bool exec() override;

 private:
  qint32 countPeaks(void);
  void reliefTangentStraightLines();
  std::pair<qint32, qreal> findMinHeight(qreal, qreal, qint32, qint32);
  qint32 findLongestInterval(void) const;
  qreal findlNull(std::pair<qint32, qreal> p);

 private:
  inline bool isTangent(qreal, qreal, qint32, qint32) const;
  inline qreal distanceSquare(qreal);
  inline qreal relativeDistances(qreal, qreal);
  inline qreal relativePoint(qreal, qint32);
};

}  // namespace Land

namespace Free {

/**
 * Составляющая расчета. Расчет затухания в свободном пространстве
 */
class Item : public Calc::Item {
 public:
  QSHDEF(Item);
  Item(const Data::WeakPtr &data) : Calc::Item(data) {}

 public:
  bool exec() override { return false; }
};

}  // namespace Free

namespace Air {

/**
 * Составляющая расчета. Расчет затухания в газах атмосферы
 */
class Item : public Calc::Item {
 public:
  QSHDEF(Item);
  Item(const Data::WeakPtr &data) : Calc::Item(data) {}

 public:
  virtual bool exec() override { return false; }
};

}  // namespace Air

}  // namespace Atten

namespace Median {

/**
 * Составляющая расчета. Расчет медианного уровня сигнала на входе приемника
 */
class Item : public Calc::Item {
 public:
  QSHDEF(Item);
  Item(const Data::WeakPtr &data) : Calc::Item(data) {}

 public:
  bool exec() override { return false; }
};

}  // namespace Median

namespace Main {

Item::Item(const Data::WeakPtr &data, QCustomPlot *cp)
    : Master::Item(data, cp) {
  _items = {
      Fill::Item::Ptr::create(_data, _cp),
      Profile::Item::Ptr::create(_data, _cp),
      Interval::Item::Ptr::create(_data), Atten::Land::Item::Ptr::create(_data),
      /*Atten::Free::Item::Ptr::create(_data),
      Atten::Air::Item::Ptr::create(_data),
      Median::Item::Ptr::create(_data)*/
  };
}

bool Item::exec() {
  for (Calc::Item::Ptr item : _items) {
    if (!item->exec()) {
      return false;
    }
  }
  return true;
}

}  // namespace Main

bool Profile::Item::exec() {
  return (Axes::Ptr::create(_data, _cp)->exec() &&
          Earth::Ptr::create(_data, _cp)->exec() &&
          Curve::Ptr::create(_data, _cp)->exec() &&
          Fresnel::Ptr::create(_data, _cp)->exec() &&
          Los::Ptr::create(_data, _cp)->exec());
}

bool Atten::Land::Item::exec() {
  switch (data->interval_type) {  // 1-Открытый, 2-Полуоткрытый, 3-Закрытый
    case 1:
      Opened::Ptr::create(_data)->exec();
      break;
    case 2:
      SemiOpened::Ptr::create(_data)->exec();
      break;
    case 3:
      Closed::Ptr::create(_data)->exec();
      break;
    default:
      return false;
  }
  return true;
}

void setGraph(QCustomPlot *cp, const QVector<double> &x,
              const QVector<double> &y, QPen pen = QPen{}) {
  static int it = 0;
  cp->addGraph();
  cp->graph(it)->setData(x, y, true);
  cp->graph(it++)->setPen(pen);
}

double Calc::Item::k(double R) {
  return R / (_data.toStrongRef()->constant.area_length);
}

double Calc::Item::lNull(double h0, double k) {
  return ((_data.toStrongRef()->constant.area_length) * qSqrt(1 + h0 * h0)) /
         (1 + (h0 * h0) / (4 * k * (1 - k)));
}

double Calc::Item::HNull(int i) {
  auto data = _data.toStrongRef();
  return sqrt(data->constant.area_length * data->constant.lambda *
              k(i * data->param.diff) * ((1 - k(i * data->param.diff)) / 3));
}

double Calc::Item::obstacleSphereRadius(double l0, double delta_y) {
  return ((l0 * l0) / (8 * delta_y)) * 0.001;
}

bool Fill::Item::exec() {
  QFile file("heights.csv");
  QTextStream in(&file);
  QRegExp rx("[ ;]");
  bool first = true;
  auto &heights = data->param.heights;
  auto &diff = data->param.diff;
  size_t &count = data->param.count;
  count = 0;

  system("sed -i '/^\s*$/d' heights.csv");
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    std::cerr << "Couldn't open heights file\n";
    return false;
  }
  while (!in.atEnd()) {
    QString line = in.readLine();
    if (first) {
      first = false;
      continue;
    }
    count++;
    line.replace(",", ".");
    heights.push_back(line.section(";", 3, 3).toDouble());
  }
  file.close();
  if (!count) {
    std::cerr << "File is empty";
    return false;
  }
  diff = (data->constant.area_length) / count;
  data->tower.sender.first = 0;
  data->tower.reciever.first = 33700;
  auto y1 = data->tower.sender.second = 117.49;
  auto y2 = data->tower.reciever.second = 52.7;
  auto y_diff = (y2 - y1) / data->param.heights.size();
  for (size_t i = 0; i + 1 <= count; ++i) {
    data->param.los_heights.push_back(y1 + i * y_diff);
    data->param.H.push_back(data->param.los_heights.at(i) -
                            data->param.heights.at(i));
    data->param.H_null.push_back(HNull(i));
    data->param.h_null.push_back(data->param.H.at(i) /
                                 data->param.H_null.at(i));
    data->param.HNull_hNull_div.push_back(data->param.H_null.at(i) /
                                          data->param.h_null.at(i));
  }
  data->param.H_null[data->param.count - 1] = 0;

  if (_data.isNull()) return false;
  return true;
}

bool Profile::Axes::exec() {
  auto data = _data.toStrongRef();
  QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
  QString str;
  QFont pfont("Arial", 8);
  _cp->rescaleAxes();
  _cp->yAxis->scaleRange(2);
  _cp->setInteractions(QCP::iSelectPlottables);
  for (double i = 0; i < data->constant.area_length + 500; i += 500) {
    str = QString::number(static_cast<int>(i) / 1000);
    textTicker->addTick(i, str);
  }
  double h_max = *std::max_element(data->param.heights.begin(),
                                   data->param.heights.end()) +
                 35;
  _cp->yAxis->setRange(0, h_max, Qt::AlignLeft);
  _cp->xAxis->setTicker(textTicker);
  _cp->xAxis->setTickLabelFont(pfont);
  _cp->xAxis->setTickLabelRotation(-45);
  _cp->xAxis->setTickLength(0);
  _cp->yAxis->setSubTickLength(0);
  _cp->xAxis->setRange(0, data->constant.area_length);
  if (_data.isNull()) return false;
  return true;
}

bool Profile::Earth::exec() {
  QVector<double> x, y;
  bool first = true;
  double move_graph_up_value;
  double equivalent_radius =
      data->constant.radius /
      (1 + data->constant.g_standard * data->constant.radius / 2);
  QPen pen(QColor("#014506"));
  pen.setWidth(2);
  for (double i = -data->constant.r_evaluated, iterator = 0;
       i < data->constant.r_evaluated;
       i += data->param.diff, iterator += data->param.diff) {
    x.push_back(i + data->constant.r_evaluated);
    if (first) {
      move_graph_up_value = qAbs(i * i / (2 * equivalent_radius));
      y.push_back(0);
      first = false;
    } else
      y.push_back(-(i * i / (2 * equivalent_radius)) + move_graph_up_value);
  }
  setGraph(_cp, x, y, pen);
  QCPItemTracer *tracer = new QCPItemTracer(_cp);
  tracer->setGraph(_cp->graph(0));
  tracer->updatePosition();
  assert(x.size() == y.size());
  if (!_data) return false;
  return true;
}

bool Profile::Curve::exec() {
  QVector<double> x, y;
  double i = 0;
  QPen pen(QColor("#137ea8"));
  pen.setWidth(2);
  for (auto h : data->param.heights) {
    x.push_back(i);
    y.push_back(h);
    i += _data.toStrongRef()->param.diff;
  }
  assert(x.size() == y.size());
  setGraph(_cp, x, y, pen);
  //    lineOfSight(plot);
  //    data->heights = heights;
  if (!_data) return false;
  return true;
}

bool Profile::Fresnel::exec() {
  QVector<double> x, y;
  QPen pen(Qt::red);
  pen.setWidth(2);
  for (size_t i = 0; i + 1 <= data->param.count; ++i) {
    x.push_back(i * data->param.diff);
    y.push_back(-data->param.H_null.at(i) + data->param.los_heights.at(i));
  }
  y[data->param.count - 1] = data->tower.reciever.second;
  setGraph(_cp, x, y, pen);
  std::for_each(y.begin(), y.end() - 1, [&](double &x) {
    static int i = 0;
    x += 2 * data->param.H_null.at(i);
    i++;
  });
  y[data->param.count - 1] = data->tower.reciever.second;
  setGraph(_cp, x, y, pen);
  if (!_data) return false;
  return true;
}

bool Profile::Los::exec() {
  QVector<double> y;
  QPen pen(QColor("#d6ba06"));
  pen.setWidth(2);
  QCPItemLine *line = new QCPItemLine(_cp);
  line->setPen(pen);
  line->start->setCoords(data->tower.sender.first, data->tower.sender.second);
  line->end->setCoords(data->tower.reciever.first, data->tower.reciever.second);
  if (!_data) return false;
  return true;
}

bool Interval::Item::exec() {
  std::cout << data->param.count;
  for (size_t i = 0; i + 1 <= data->param.count; ++i) {
    if (data->param.H.at(i) >= data->param.H_null.at(i) &&
        data->param.h_null.at(i) >= 0)
      data->interval_type = std::max(data->interval_type, 1);
    else if (data->param.H_null.at(i) > data->param.H.at(i) &&
             data->param.H.at(i) > 0.1 && data->param.h_null.at(i) < 0.1 &&
             data->param.h_null.at(i) > 0)
      data->interval_type = std::max(data->interval_type, 2);
    else if (data->param.H_null.at(i) > data->param.H.at(i) &&
             data->param.h_null.at(i) < 0)
      data->interval_type = 3;
  }
  if (!_data) return false;
  return true;
}

std::pair<double, double> Atten::Land::Item::strLineEquation(
    double x, double y, double x_relief, double y_relief) const {
  double c = (y_relief - y);
  double d = (x_relief - x);
  return {c / d, y - (x * c / d)};
}

// Составляющая расчета. Реализация расчета затухания на открытом интервале

bool Atten::Land::Opened::exec() { findPointOfIntersection(); }

void Atten::Land::Opened::findPointOfIntersection(void) {
  int int_start = 0;
  int int_end = data->param.count - 1;
  double opposite_y_coord =
      2 * data->param.heights.at(int_start) - data->tower.sender.second;
  auto [a, b] =
      strLineEquation(int_start, opposite_y_coord, int_end * data->param.diff,
                      data->tower.reciever.second);
  double intersection_index = -1;
  double max_height = 0;
  for (auto it = int_start; it < int_end;
       ++it) {  // поиск точки пересечения, если их несколько, то берется
                // наивысшая точка
    auto y_coord = a * it * data->param.diff + b;
    if ((y_coord >= data->param.heights.at(it) &&
         y_coord <= data->param.heights.at(it + 1)) ||
        (y_coord <= data->param.heights.at(it) &&
         y_coord >= data->param.heights.at(it + 1))) {
      if (y_coord > max_height) {
        max_height = y_coord;
        intersection_index = it;
      }
    }
  }
  if (intersection_index == -1) {
    qDebug() << "Bad opened interval. Terminating.";
    exit(EXIT_FAILURE);
  }
  double l_null_length = lNull(data->param.h_null.at(intersection_index),
                               k(intersection_index * data->param.diff));
  int_start = std::max(
      int_start,
      static_cast<int>(intersection_index - l_null_length / data->param.diff));
  int_end = std::min(
      int_end,
      static_cast<int>(intersection_index + l_null_length / data->param.diff));
  if (l_null_length <= 0.25 * data->constant.area_length)
    PlaneApproximation();
  else
    SphereApproximation();
}

void Atten::Land::Opened::PlaneApproximation() { rayleighAndGroundCriteria(); }

void Atten::Land::Opened::SphereApproximation() {
  auto h = data->param.heights;
  auto min_height =
      *std::min_element(h.begin() + int_start, h.begin() + int_end);
  auto max_height =
      *std::max_element(h.begin() + int_start, h.begin() + int_end);
  auto delta_y = qAbs(max_height - min_height);
  if (max_height == min_height) return;
  //  qDebug() << obstacleSphereRadius(
  //      (int_end - int_start) * data->intervals_difference, delta_y);
}

void Atten::Land::Opened::rayleighAndGroundCriteria() {
  double k, b, denominator, y, delta_h;
  auto h = data->param.heights;
  auto [a, z] = lineOfSightCoords();
  denominator = (z - a);
  k = (double(h.at(z) - h.at(a)) / denominator);
  b = h.at(a) - (a * (h.at(z) - h.at(a))) / denominator;
  auto max_H0_h0 =
      *std::max_element(data->param.HNull_hNull_div.begin() + int_start,
                        data->param.HNull_hNull_div.begin() + int_end);
  for (auto i = int_start; i <= int_end; ++i) {
    y = k * i + b;
    delta_h = abs(y - h.at(i));
    //        qDebug() << delta_h << 0.75 * max_H0_h0;
    //    if (delta_h < 0.75 * max_H0_h0) {
    //      qDebug() << i << ": Гладкая";
    //      qDebug() << attentuationPlane(delta_h);
    //    } else
    //      qDebug() << '-';
  }
}

double Atten::Land::Opened::findEl(int piv, int limit) {
  auto h = data->param.heights;
  auto a = qAbs(piv - limit);
  for (int i = 0; i < a; ++i) {
    int ind;
    if (limit > piv)
      ind = i + piv;
    else
      ind = piv - i;
    if (h.at(ind - 1) <= h.at(ind) && h.at(ind + 1) <= h.at(ind)) {
      return ind;
    }
  }
}

std::pair<int, int> Atten::Land::Opened::lineOfSightCoords() {
  int pivot = (int_end + int_start) / 2;
  double max_first = int_start, max_last = int_end;
  max_last = findEl(pivot, int_end);
  max_first = findEl(pivot, int_start);
  return {max_first, max_last};
}

qreal Atten::Land::Opened::attentuationPlane(qreal delta_h) {
  return -10 * log10(2 - 2 * qCos(M_PI / 3 * delta_h * delta_h));
}  // Конец реализации расчета затухания на открытом интервале

// Составляющая расчета. Реализация расчета затухания на полуоткрытом интервале

bool Atten::Land::SemiOpened::exec() {
  approx();
  if (!_data) return false;
  return true;
}

void Atten::Land::SemiOpened::approx() {
  qreal l_null_length;
  auto index_H = shadingObstacle();
  l_null_length = lNull(data->param.h_null.at(index_H.first),
                        k(index_H.first * data->param.diff));
  //  if (!uniteObstacles(int_start, int_end)) {
  qreal a = obstacleSphereRadius(l_null_length, deltaY());
  int_end = a;
  if (a >= qSqrt(data->constant.area_length * data->constant.lambda * 0.5 *
                 (0.5 / 3))) {
    SphereApproximation(index_H.first, a);
  } else {
    WedgeApproximation(index_H.first);
  }
}

std::pair<qreal, qreal> Atten::Land::SemiOpened::SphereApproximation(
    int idx, double obst_sph_radius) {
  qreal k_avg = k(idx * data->param.diff);
  qreal mu =
      areaReliefParameter(k_avg, data->param.H_null.at(idx), obst_sph_radius);
  return {reliefParFuncSph(mu), attenuationPSph(mu)};
}

// Аппроксимация клином
qreal Atten::Land::SemiOpened::WedgeApproximation(int idx) {
  qreal k_avg = k(idx * data->param.diff);
  qreal nu = nuWedg(data->param.H.at(idx), k_avg);
  return attentuationPWedg(nu);
}

qreal Atten::Land::SemiOpened::deltaY() {
  QVector<qint32> intersec_heights;
  auto y_diff = qAbs(data->tower.reciever.second - data->tower.sender.second) /
                data->param.heights.size();
  for (size_t ind = 0; ind + 1 <= data->param.count; ++ind) {
    auto a = static_cast<qint32>(data->param.heights.at(ind));
    auto b = static_cast<qint32>(data->tower.sender.second + ind * y_diff -
                                 data->param.H_null.at(ind));
    if (b >= a - 1 && b <= a + 1) intersec_heights.push_back(ind);
  }
  if (intersec_heights.size() == 1 || !intersec_heights.size()) {
    return 0;
  }
  auto right = *std::lower_bound(intersec_heights.begin(),
                                 intersec_heights.end(), int_start);
  auto left =
      *std::lower_bound(intersec_heights.rbegin(), intersec_heights.rend(),
                        int_end, [](qint32 a, qint32 b) { return a > b; });
  if (right == left)
    right = *std::lower_bound(intersec_heights.begin(), intersec_heights.end(),
                              right);
  return abs(data->param.heights.at(right) - data->param.heights.at(left));
}

std::pair<qint32, qreal> Atten::Land::SemiOpened::shadingObstacle(void) const {
  auto it = std::min_element(data->param.H.begin(), data->param.H.end());
  qreal dist = std::distance(data->param.H.begin(), it);  // индекс препятствия
  return {dist, *it};
}

double Atten::Land::SemiOpened::areaReliefParameter(
    double k, double H0, double obstacleShereRadius) {
  return pow(qPow(data->constant.area_length, 2) * k * k * ((1 - k) * (1 - k)) /
                 (obstacleShereRadius * H0),
             1.0 / 3);
}

double Atten::Land::SemiOpened::reliefParFuncSph(double mu) {
  return 4 + 10 / (mu - 0.1);
}

double Atten::Land::SemiOpened::attenuationPSph(double mu) {
  return 6 + 16.4 / (mu * (1 + 0.8 * mu));
}

double Atten::Land::SemiOpened::nuWedg(double H, double k) {
  return -H * qSqrt(2 / (data->constant.lambda * data->constant.r_evaluated *
                         2 * k * (1 - k)));
}

double Atten::Land::SemiOpened::attentuationPWedg(double nu) {
  return 6.9 + 20 * log10(qSqrt(pow(nu - 0.1, 2) + 1) + nu - 0.1);
}

double Atten::Land::SemiOpened::t(double l, double s) { return l / s; }

double Atten::Land::SemiOpened::uniteApprox(double R1, double R2) {
  return log10(M_PI - qAsin(qSqrt(data->constant.area_length * (R2 - R1) /
                                  (R2 * (data->constant.area_length - R1)))));
}  // Конец реализации расчета затухания на полуоткрытом интервале

// Составляющая расчета. Реализация расчета затухания на закрытом интервале

bool Atten::Land::Closed::exec() {
  //  countPeaks();
  //  reliefTangentStraightLines();
  if (!_data) return false;
  return true;
}

qint32 Atten::Land::Closed::countPeaks(void) {
  qint32 i = 0, count = 0;
  bool inside = 0;
  for (auto it : data->param.los_heights) {
    auto next = std::min(i + 1, data->param.heights.size() - 1);
    if (it >= data->param.heights.at(i) && it <= data->param.heights.at(next) &&
        inside == 0) {
      inside = 1;
    } else if (it >= data->param.heights.at(next) &&
               it <= data->param.heights.at(i) && inside == 1) {
      inside = 0;
      count++;
    }
    i++;
  }
  return count;
}

void Atten::Land::Closed::reliefTangentStraightLines() {
  std::pair<qint32, qreal> min_height_send, min_height_rec;
  qreal ind_send, ind_rec;  // индексы точек касания

  for (auto i = int_start; i <= int_end; ++i) {
    if (i == int_start || i == int_end) continue;
    auto [a_sender, b_sender] =
        strLineEquation(data->tower.sender.first, data->tower.sender.second,
                        i * data->param.diff, data->param.heights.at(i));
    auto [a_reciever, b_reciever] =
        strLineEquation(data->tower.reciever.first, data->tower.reciever.second,
                        i * data->param.diff, data->param.heights.at(i));
    if (isTangent(a_sender, b_sender, int_start, int_end)) {
      ind_send = i;
      min_height_send = findMinHeight(a_sender, b_sender, int_start, int_end);
    }
    if (isTangent(a_reciever, b_reciever, int_start, int_end)) {
      ind_rec = i;
      min_height_rec =
          findMinHeight(a_reciever, b_reciever, int_start, int_end);
    }
  }

  auto [x, p] = (min_height_send.second < min_height_rec.second)
                    ? std::make_pair(ind_send, min_height_send)
                    : std::make_pair(ind_rec, min_height_rec);
  qreal delta_y = qAbs(data->param.heights.at(x) - p.second);
  qreal a = obstacleSphereRadius(findlNull(p) * data->param.diff, delta_y);
  qreal s = distanceSquare(a);
  std::cout << s;
  std::cout << relativeDistances(s, min_height_send.first * data->param.diff);
  std::cout << relativeDistances(
      s, qAbs(data->param.heights.size() - min_height_rec.first - 1));
  //  qDebug() << s;
  //  qDebug() << relativeDistances(s, min_height_send.first * dist);
  //  qDebug() << relativeDistances(
  //      s, qAbs(s_data->heights.size() - min_height_rec.first - 1));
}

bool Atten::Land::Closed::isTangent(qreal a, qreal b, qint32 int_start,
                                    qint32 int_end) const {
  for (qint32 j = int_start; j <= int_end; ++j) {
    if (a * j * data->param.diff + b < data->param.heights.at(j)) return false;
  }
  return true;
}

std::pair<qint32, qreal> Atten::Land::Closed::findMinHeight(qreal a, qreal b,
                                                            qint32 start,
                                                            qint32 end) {
  auto min_height =
      std::max_element(data->param.heights.begin(), data->param.heights.end());
  qint32 height_index = 0;
  QVector<qreal> x, y;
  for (qint32 i = start, k = 0; i <= end; ++i, ++k) {
    x.push_back(i * data->param.diff);
    y.push_back(a * i * data->param.diff + b - qAbs(data->param.H_null.at(i)));

    if (y.at(k) < data->param.heights.at(i) + 1 &&
        y.at(k) > data->param.heights.at(i) - 1) {
      *min_height = std::min(*min_height, data->param.heights.at(i));
      height_index = i;
    }
  }
  return {height_index, *min_height};
}

qreal Atten::Land::Closed::findlNull(
    std::pair<qint32, qreal> p) {  // TODO: нужен больший обзор
  auto it = std::find_if(data->param.heights.begin() + int_start,
                         data->param.heights.begin() + int_end, [=](int val) {
                           return (val + 1 > p.second) && (val - 1 < p.second);
                         });
  return qAbs(std::distance(it, data->param.heights.begin() + p.first));
}

qreal Atten::Land::Closed::distanceSquare(qreal obstSphRadius) {
  return qPow((obstSphRadius * obstSphRadius * data->constant.lambda) / M_PI,
              0.33333);
}

qreal Atten::Land::Closed::relativeDistances(qreal s, qreal r) { return s / r; }

qreal Atten::Land::Closed::relativePoint(qreal a, qint32 i) {
  auto z = k(i * data->param.diff);
  return (a * data->param.H.at(i)) / (data->constant.area_length * z * (1 - z));
}  // Конец реализации расчета затухания на закрытом интервале

Core::Core(QCustomPlot *cp) {
  _data = QSharedPointer<Data>::create();
  _cp = cp;
  _main = Main::Item::Ptr::create(_data, _cp);
}

bool Core::exec() { return _main->exec(); }

}  // namespace Calc

}  // namespace NRrls
