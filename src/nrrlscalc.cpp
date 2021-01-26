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

  // NRrls::Calc::Item interface
 public:
  bool exec() override;

 private:
  QSharedPointer<NRrls::Calc::Data> data = _data.toStrongRef();
};

}  // namespace Fill

namespace Profile {

/*
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

/*
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
 * Составляющая расчета. Определение интервала. Базовый класс
 */
class Item : public Calc::Item {
 public:
  QSHDEF(Item);
  Item(const Data::WeakPtr &data) : Calc::Item(data) {}

 public:
  virtual bool exec() override;

 protected:
  QSharedPointer<NRrls::Calc::Data> data = _data.toStrongRef();
  int interval_type;  ///< Тип интервала: 1-Открытый, 2-Полуоткрытый, 3-Закрытый
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
  double attentuationPlane(double);
};

/**
 * Составляющая расчета. Расчет затухания на открытом интервале
 */
class SemiOpened : public Item {
 public:
  QSHDEF(Item);
  SemiOpened(const Data::WeakPtr &data) : Item(data) {}

 public:
  bool exec() override;

 private:
  void approx(void);
  std::pair<double, double> SphereApproximation(void);
  double WedgeApproximation(void);
  double deltaY(void);
  std::pair<int, double> shadingObstacle(void) const;
};

/**
 * Составляющая расчета. Расчет затухания на открытом интервале
 */
class Closed : public Item {
 public:
  QSHDEF(Item);
  Closed(const Data::WeakPtr &data) : Item(data) {}

 public:
  bool exec() override;
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
      Interval::Item::Ptr::create(_data),
      /*Atten::Land::Item::Ptr::create(_data),
      Atten::Free::Item::Ptr::create(_data),
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

void setGraph(QCustomPlot *cp, const QVector<double> &x,
              const QVector<double> &y, QPen pen = QPen{}) {
  static int it = 0;
  cp->addGraph();
  cp->graph(it)->setData(x, y, true);
  cp->graph(it++)->setPen(pen);
}

double Calc::Item::k(double R) {
  return R / (2 * _data.toStrongRef()->constant.r_evaluated);
}

double Calc::Item::lNull(double h0, double k) {
  return ((_data.toStrongRef()->constant.r_evaluated * 2) *
          qSqrt(1 + h0 * h0)) /
         (1 + (h0 * h0) / (4 * k * (1 - k)));
}

double Calc::Item::HNull(int i) {
  auto data = _data.toStrongRef();
  return sqrt(data->constant.r_evaluated * 2 * data->constant.lambda *
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
  diff = (data->constant.r_evaluated * 2) / count;
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

bool Profile::Item::exec() {
  return (Axes::Ptr::create(_data, _cp)->exec() &&
          Earth::Ptr::create(_data, _cp)->exec() &&
          Curve::Ptr::create(_data, _cp)->exec() &&
          Fresnel::Ptr::create(_data, _cp)->exec() &&
          Los::Ptr::create(_data, _cp)->exec());
}

bool Profile::Axes::exec() {
  auto data = _data.toStrongRef();
  QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
  QString str;
  QFont pfont("Arial", 8);
  _cp->rescaleAxes();
  _cp->yAxis->scaleRange(2);
  _cp->setInteractions(QCP::iSelectPlottables);
  for (double i = 0; i < data->constant.r_evaluated * 2 + 500; i += 500) {
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
  _cp->xAxis->setRange(0, data->constant.r_evaluated * 2);
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
  //    s_data->heights = heights;
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
  for (size_t i = 0; i + 1 <= data->param.count; ++i) {
    if (data->param.H.at(i) >= data->param.H_null.at(i) &&
        data->param.h_null.at(i) >= 0)
      interval_type = std::max(interval_type, 1);
    else if (data->param.H_null.at(i) > data->param.H.at(i) &&
             data->param.H.at(i) > 0.1 && data->param.h_null.at(i) < 0.1 &&
             data->param.h_null.at(i) > 0)
      interval_type = std::max(interval_type, 2);
    else if (data->param.H_null.at(i) > data->param.H.at(i) &&
             data->param.h_null.at(i) < 0)
      interval_type = 3;
  }
  if (!_data) return false;
  return true;
}

bool Atten::Land::Item::exec() {
  switch (interval_type) {
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

bool Atten::Land::Opened::exec() { findPointOfIntersection(); }

std::pair<double, double> Atten::Land::Item::strLineEquation(
    double x, double y, double x_relief, double y_relief) const {
  double c = (y_relief - y);
  double d = (x_relief - x);
  return {c / d, y - (x * c / d)};
}

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
  if (l_null_length <= 0.25 * 2 * data->constant.r_evaluated)
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
  //      (int_end - int_start) * s_data->intervals_difference, delta_y);
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
}

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
  if (a >= qSqrt(data->constant.r_evaluated * 2 * data->constant.lambda * 0.5 *
                 (0.5 / 3))) {
    SphereApproximation();
  } else {
    WedgeApproximation();
  }
}

std::pair<qreal, qreal>
Atten::Land::SemiOpened::SphereApproximation() {
  qreal k_avg = k(idx * s_data->intervals_difference);
  qreal mu =
      areaReliefParameter(k_avg, s_data->H_null.at(idx), obst_sph_radius);
  return {reliefParFuncSph(mu), attenuationPSph(mu)};
}

// Аппроксимация клином
qreal Atten::Land::SemiOpened::WedgeApproximation() {
  qreal k_avg = k(idx * s_data->intervals_difference);
  qreal nu = nuWedg(s_data->H.at(idx), k_avg);
  return attentuationPWedg(nu);
}

Core::Core(QCustomPlot *cp) {
  _data = QSharedPointer<Data>::create();
  _cp = cp;
  _main = Main::Item::Ptr::create(_data, _cp);
}

bool Core::exec() { return _main->exec(); }

}  // namespace Calc

}  // namespace NRrls
