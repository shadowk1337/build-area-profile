#include "nrrlscalc.h"

QTextStream ostream(stdout);
QTextStream estream(stderr);

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
  void paramFill(void);

 private:
  QSharedPointer<Calc::Data> data = _data.toStrongRef();
};

}  // namespace Fill

namespace Profile {

/**
 * Составляющая расчета. Построение графика
 */
class Item : public Calc::Item {
 public:
  QSHDEF(Item);
  Item(const Calc::Data::WeakPtr &data, QCustomPlot *cp)
      : Calc::Item(data, cp) {}

 public:
  virtual bool exec() override;

 protected:
  QSharedPointer<Calc::Data> data = _data.toStrongRef();
  decltype(data->param.coords) coords = data->param.coords;
};

/**
 * Составляющая расчета. Задание осей графика
 */
class Axes : public Profile::Item {
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
class Earth : public Profile::Item {
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
class Curve : public Profile::Item {
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
class Fresnel : public Profile::Item {
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
class Los : public Profile::Item {
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
  bool exec() override;

 protected:
  QSharedPointer<NRrls::Calc::Data> data = _data.toStrongRef();
};

}  // namespace Interval

namespace Atten {

namespace Land {

/**
 * Составляющая расчета. Расчет затухания в рельефе
 */
class Item : public Calc::Item {
 public:
  QSHDEF(Item);
  Item(const Data::WeakPtr &data) : Calc::Item(data) {}

 public:
  virtual bool exec() override;

 protected:
  class Edge {
   public:
    Edge() { _p = qMakePair(-1, -1); }
    Edge(double x, double y) { _p = qMakePair(x, y); }

    double x(void) { return _p.first; }
    double y(void) { return _p.second; }

   private:
    QPair<double, double> _p;
  };
  /**
   * Функция проверки касательности прямой к высотному профилю
   * @param a       - y = a * x + b
   * @param b       - y = a * x + b
   * @param start   - начальный индекс рассматриваемого интервала
   * @param end     - конечный индекс рассматриваемого интервала
   * @return Будет ли прямая касательной
   */
  bool _isTangent(double a, double b, double start, double end) const;

  QSharedPointer<Calc::Data> data = _data.toStrongRef();
  decltype(data->param.coords) coords = data->param.coords;
  Edge calc_start =
      Edge(coords.begin().key(),
           coords.begin().value());  ///< Координата начальной точки интервала
  //  Edge calc_end =
  //      Edge(std::prev(coords.end()).key(),
  //           coords.end().value()); ///< Координата конечной точки интервала
};

/**
 * Составляющая расчета. Расчет затухания на открытом интервале
 */
class Opened : public Land::Item {
 public:
  QSHDEF(Opened);
  typedef QMap<double, double>::ConstIterator QMapCIt;
  Opened(const Data::WeakPtr &data) : Land::Item(data) {}

 public:
  bool exec() override;

 private:
  /**
   * Функция нахождения наивысшей точки пересечения высотного профиля и
   * отрезка, соединяющего приемник и точку, зеркальную передатчику
   * относительно высотного профиля
   * @return Координаты наивысшей точки пересечения
   */
  QPair<double, double> _findPointOfIntersection(void);

  /**
   * Функция аппроксимации плоскостью
   * @param start   - Координаты начальной точки участка отражения
   * @param end     - Координаты конечной точки участка отражения
   */
  void _planeApproximation(QMapCIt start, QMapCIt end);

  /**
   * Функция аппроксимации сферой
   * @param start   - Координаты начальной точки участка отражения
   * @param end     - Координаты конечной точки участка отражения
   */

  QPair<QMap<double, double>::ConstIterator,
        QMap<double, double>::ConstIterator>
  _findIntersectionInterval(void);

  bool _isTangent(double a, double b) const;

  /**
   * Функция сравнения с критерием Рэлея
   * @param start   - Координаты начальной точки участка отражения
   * @param end     - Координаты конечной точки участка отражения
   */
  void _rayleighAndGroundCriteria(QMapCIt mid, QMapCIt length);

 private:
  double _attentuationPlane(double phi_null, double delta_h);

  double _mirror_coef(double delta_h) {
    //    double chi = .32 * delta_h * return std::max(qExp(-(qPow(4 * M_PI
    //    *))))
  }
};

/**
 * Составляющая расчета. Расчет затухания на полуоткрытом интервале
 */
class SemiOpened : public Land::Item {
 public:
  QSHDEF(SemiOpened);
  SemiOpened(const Data::WeakPtr &data) : Land::Item(data) {}

 public:
  bool exec() override;

 private:
  /**
   * Функция аппроксимации сферой
   * @param x     - индекс рассматриваемой точки
   * @param obst_sph_radius
   *                - радиус препятствия
   * @return Затухание на интервале
   */
  double _sphereApproximation(double x, double obst_sph_radius);

  /**
   * Функция аппроксимации клином
   * @param x     - индекс рассматриваемой точки
   * @return Затухание на интервале
   */
  double _wedgeApproximation(double x);

  /**
   * Функция нахождения высоты хорды
   * @param sh_height
   *                - высота затеняющего препятствия
   * @return Величина высоты хорды
   */
  double _deltaY(double sh_ind);

  /**
   * Функция нахождения затеняющего препятствия
   * @return Пара .first
   *                - индекс препятствия .second - величина минимального
   * просвета
   */
  QPair<double, double> _shadingObstacle(void) const;

 private:
  /**
   * Функция нахождения параметра рельефа местности
   * @param k       - относительная координата
   * @param H0      - критический просвет
   * @param obstacleShereRadius
   *                - радиус сферы препятствия
   * @return Величина параметра рельефа местности
   */
  inline double _areaReliefParameter(double k, double H0,
                                     double obstacleShereRadius);

  /**
   * Функция нахождения функции параметра рельефа
   * @param mu      - параметр рельефа местности
   * @return Величина функции параметра рельефа
   */
  inline double _reliefParFuncSph(double mu);

  /**
   * Функция нахождения затухания
   * @param mu      - параметр рельефа местности
   * @return Величина затухания
   */
  inline double _attentuationPSph(double mu);

  /**
   * Функция нахождения параметра клиновидного препятствия
   * @param H       - реальный просвет
   * @param k       - относительная координата
   * @return Величина параметра клиновидного препятствия
   */
  inline double _nuWedg(double H, double k);

  /**
   * Функция нахождения затухания
   * @param nu      - параметр клиновидного препятствия
   * @return Величина затухания
   */
  inline double _attentuationPWedg(double nu);

  /**
   * Функция нахождения относительной протяженности вершины препятствия
   * @param l       - действительная протяженность вершины
   * @param s       - масштаб расстояний
   * @return Величина относительной протяженности вершины препятствия
   */
  inline double _t(double l, double s);
};

/**
 * Составляющая расчета. Расчет затухания на закрытом интервале
 */
class Closed : public Land::Item {
 public:
  QSHDEF(Closed);
  typedef QList<QPair<double, double>> Peaks;
  Closed(const Data::WeakPtr &data) : Land::Item(data) {}

 public:
  bool exec() override;

 private:
  /**
   * Функция подсчета препятствий
   * @return Отрезки, содержащие препятствия
   */
  auto _countPeaks(void) -> Peaks;

  /**
   * Функция аппроксимации одним эквивалентом
   * @param v       - индексы затеняющих препятствий
   */
  void _approx(Peaks &v);

  /**
   * Функция нахождения прямых, касательных высотному профилю на отрезке [start,
   * end]
   * @param start   - индекс начальной точки
   * @param end     - индекс конечной точки
   */
  double _reliefTangentStraightLines(const Peaks &p);

  /**
   * Функция построения уравнений прямых и проверки касательности их к высотному
   * профилю
   * @param p       - кортеж, в который будут передаваться данные
   * @param start   - индекс входа ЛПВ в препятствие
   * @param end     - индекс выхода ЛПВ из препятствия
   * @param type    - передатчик/приемник
   */
  double _tangent(const QPair<double, double> &p,
                  const QPair<double, double> &left,
                  const QPair<double, double> &right);

  /**
   * @brief findlNull
   * @param p
   * @return
   */
  double _findlNull(std::pair<int, double> p);

 private:
  inline QPair<double, double> _pointOfIntersection(QPair<double, double> l,
                                                    QPair<double, double> r);

  inline double _diffractionParam(double h, double d1, double d2);

  inline double _atten(double param);
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
      qMakePair(Fill::Item::Ptr::create(_data, _cp), QString("Fill")),
      qMakePair(Profile::Item::Ptr::create(_data, _cp), QString("Profile")),
      qMakePair(Interval::Item::Ptr::create(_data), QString("Interval")),
      qMakePair(Atten::Land::Item::Ptr::create(_data), QString("Attentuation")),
      /*Atten::Free::Item::Ptr::create(_data),
      Atten::Air::Item::Ptr::create(_data),
      Median::Item::Ptr::create(_data)*/
  };
}

bool Item::exec() {
  for (auto item : _items) {
    if (!item.first->exec()) {
      estream << "Error in " << QString("%1 %2").arg(__FILE__).arg(item.second)
              << " function\n";
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
      //    SemiOpened::Ptr::create(_data)->exec();
      break;
    case 3:
      Closed::Ptr::create(_data)->exec();
      break;
    default:
      return false;
  }
  return true;
}

bool Atten::Land::Item::_isTangent(double a, double b, double start,
                                   double end) const {
  LOOP_START(coords.lowerBound(start), coords.lowerBound(end), it);
  if (a * it.key() + b < it.value()) return false;
  LOOP_END;
  return true;
}

namespace Etc {

void setGraph(QCustomPlot *cp, const QVector<double> &x,
              const QVector<double> &y, QPen pen = QPen{}) {
  static int it = 0;
  cp->addGraph();
  cp->graph(it)->setData(x, y, true);
  cp->graph(it++)->setPen(pen);
}

}  // namespace Etc

double Item::k(double R) const {
  return R / (_data.toStrongRef()->constant.area_length);
}

double Item::lNull(double h0, double k) const {
  return ((_data.toStrongRef()->constant.area_length) * qSqrt(1 + h0 * h0)) /
         (1 + (h0 * h0) / (4 * k * (1 - k)));
}

double Item::HNull(int i) const {
  auto data = _data.toStrongRef();
  return sqrt(data->constant.area_length * data->constant.lambda * k(i) *
              ((1 - k(i)) / 3));
}

double Item::obstacleSphereRadius(double l0, double delta_y) const {
  return ((l0 * l0) / (8 * delta_y)) * 0.001;
}

QPair<double, double> Calc::Item::strLineEquation(double x, double y, double xx,
                                                  double yy) const {
  double a = (yy - y);
  double b = (xx - x);
  return {a / b, y - (x * a / b)};
}

namespace Fill {

bool Item::exec() {
  QString filename = "heights4.csv";
  QFile file(filename);
  QTextStream in(&file);
  QRegExp rx("[ ;]");
  bool first = true;
  auto &coords = data->param.coords;
  size_t &count = data->param.count;
  count = 0;

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    estream << QString("Couldn't open file %1\n").arg(filename);
    return false;
  }
#ifndef Q_OS_WINDOWS
  system("sed -i '/^*$/d' heights4.csv");
#endif
  int l = -1, h = -1;
  while (!in.atEnd()) {
    QString line = in.readLine();
    if (first) {
      first = false;
      int i = 0;
      while (!line.section(";", i, i).isNull()) {
        line = line.toLower();
        if (line.section(";", i, i).contains("расстояние"))
          l = i;
        else if (line.section(";", i, i).contains("высота"))
          h = i;
        i++;
      }
      continue;
    }
    if (l == -1 || h == -1) {
      estream << QString("File %1 doesn't consist table names\n").arg(filename);
      return false;
    }
    count++;
    line.replace(",", ".");
    coords[line.section(";", l, l).toDouble()] =
        line.section(";", h, h).toDouble();
  }
  file.close();
  if (!count) {
    estream << QString("File %1 is empty\n").arg(filename);
    return false;
  }
  paramFill();

  if (_data.isNull()) return false;
  return true;
}

void Item::paramFill(void) {
  auto &coords = data->param.coords;
  data->tower.sender.first = coords.b_x();
  data->tower.reciever.first = coords.e_x();
  data->constant.area_length = coords.e_x() - coords.b_x();
  auto y1 = data->tower.sender.second = coords.b_y() + 20;
  auto y2 = data->tower.reciever.second = coords.e_y() + 20;
  data->param.los = strLineEquation(data->tower.sender.first, y1,
                                    data->tower.reciever.first, y2);
  LOOP_START(coords.begin(), coords.end(), it);
  data->param.H[it.key()] =
      data->param.los.first * it.key() + data->param.los.second - it.value();
  data->param.H_null[it.key()] = HNull(it.key());
  data->param.h_null[it.key()] =
      (data->param.H[it.key()] / data->param.H_null[it.key()]);
  LOOP_END;
  data->param.H_null[data->param.count - 1] = 0;
}

}  // namespace Fill

namespace Profile {

bool Axes::exec() {
  constexpr int window_add_height = 35;
  constexpr int window_add_length = 100;
  QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
  QFont pfont("Arial", 8);
  _cp->rescaleAxes();
  _cp->setInteractions(QCP::iSelectPlottables);

  for (double i = 0; i < data->constant.area_length + 500; i += 500) {
    QString str = QString::number(static_cast<int>(i) / 1000);
    textTicker->addTick(i, str);
  }

  QVector<double> heights;
  coords.y(heights);
  double h_max =  ///< Максимальная высота графика
      *std::max_element(heights.begin(), heights.end()) + window_add_height;
  _cp->yAxis->scaleRange(2);
  _cp->yAxis->setRange(0, h_max, Qt::AlignLeft);
  _cp->xAxis->setTicker(textTicker);
  _cp->xAxis->setTickLabelFont(pfont);
  _cp->xAxis->setTickLabelRotation(-45);
  _cp->xAxis->setTickLength(0);
  _cp->yAxis->setSubTickLength(0);
  _cp->xAxis->setRange(0, data->constant.area_length + window_add_length);

  if (_data.isNull()) return false;
  return true;
}

bool Earth::exec() {
  QVector<double> x, y;
  RESERVE(x, y, data->param.count);
  bool first = true;
  double move_graph_up_value;
  double equivalent_radius =
      data->constant.radius /
      (1 + data->constant.g_standard * data->constant.radius / 2);
  QPen pen(QColor("#014506"));
  pen.setWidth(2);

  double half =  ///< Половина длины рассматриваемого участка
      data->constant.area_length / 2;
  //  QVector<Profile::Data::Coords>::iterator it = coords.begin();
  coords.x(x);
  decltype(x)::ConstIterator it = x.begin();
  for (double i = -half; it != x.end(); it = std::next(it), i = -half + *it) {
    if (first) {
      first = false;
      move_graph_up_value = i * i / (2 * equivalent_radius);
      y.push_back(0);
    } else {
      y.push_back(-(i * i / (2 * equivalent_radius)) + move_graph_up_value);
    }
  }
  Etc::setGraph(_cp, x, y, pen);
  assert(x.size() == y.size());

  if (!_data) return false;
  return true;
}

bool Curve::exec() {
  QVector<double> x, y;
  RESERVE(x, y, data->param.count);
  QPen pen(QColor("#137ea8"));
  pen.setWidth(2);
  coords.x(x);
  coords.y(y);
  Etc::setGraph(_cp, x, y, pen);

  if (!_data) return false;
  return true;
}

bool Fresnel::exec() {
  QVector<double> x, y;
  RESERVE(x, y, data->param.count);
  QPen pen(Qt::red, 2);

  coords.x(x);
  LOOP_START(coords.begin(), coords.end(), i);
  y.push_back((-data->param.H_null[i.key()]) + data->param.los.first * i.key() +
              data->param.los.second);
  LOOP_END;
  Etc::setGraph(_cp, x, y, pen);

  decltype(x)::ConstIterator it = x.begin();
  LOOP_START(y.begin(), y.end(), i);
  *i += 2 * data->param.H_null[*it];
  it = (it != x.end()) ? std::next(it) : std::prev(x.end());
  LOOP_END;
  Etc::setGraph(_cp, x, y, pen);
  if (!_data) return false;
  return true;
}

bool Los::exec() {
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

}  // namespace Profile

bool Interval::Item::exec() {
  LOOP_START(data->param.coords.begin(), data->param.coords.end(), it);
  auto i = it.key();
  if (data->param.H[i] >= data->param.H_null[i] && data->param.h_null[i] >= 0)
    data->interval_type = std::max(data->interval_type, 1);
  else if (data->param.H_null[i] > data->param.H[i] && data->param.H[i] > 0.1 &&
           data->param.h_null[i] < 0.1 && data->param.h_null[i] > 0)
    data->interval_type = std::max(data->interval_type, 2);
  else if (data->param.H_null[i] > data->param.H[i] &&
           data->param.h_null[i] < 0)
    data->interval_type = 3;
  LOOP_END;
  if (!_data) return false;
  return true;
}

namespace Atten {
namespace Land {

// Составляющая расчета. Реализация расчета затухания на открытом интервале

bool Opened::exec() {
  auto p = _findIntersectionInterval();
  //  ostream << p.first.key() << ' ' << p.second.key();
  if (!_data) return false;
  return true;
}

QPair<QMap<double, double>::ConstIterator, QMap<double, double>::ConstIterator>
Opened::_findIntersectionInterval(void) {
  auto los_parallel = data->param.los;
  QMap<double, double>::ConstIterator it;
  QMap<double, double>::ConstIterator H_min = data->param.H.begin();
  for (it = data->param.H.begin(); it != data->param.H.end(); ++it)
    //    H_min = H_min.value() > it.value() ?
    //    std::lower_bound(data->param.H.begin(), data->param.H.end(), it->) :
    //    H_min;
    while (!_isTangent(los_parallel.first, los_parallel.second)) {
      it = std::next(it);
      los_parallel.second = it.value() - los_parallel.first * it.key();
    }
  los_parallel.second -= data->param.H_null[it.key()];
  ostream << it.key() << ' ' << it.value();
  //  for (auto i = it; i != coords.end(); ++i) {
  //    if ((i + 1).value() <
  //        los_parallel.first * (i + 1).key() + los_parallel.second) {
  //      for (auto j = it; j != coords.begin(); --j) {
  //        if ((i - 1).value() <
  //            los_parallel.first * (i - 1).key() + los_parallel.second)
  //          return {i, j};
  //      }
  //    }
  //  }
}

void Opened::_rayleighAndGroundCriteria(QMapCIt start, QMapCIt end) {
  QVector<double> h;
  coords.y(h);
  auto pair =
      strLineEquation(start.key(), start.value(), end.key(), end.value());
  auto H_null = data->param.H_null.values();
  auto h_null = data->param.h_null.values();
  auto max_H0_h0 =  ///< Максимально допустимое значение неровности рельефа
      0.75 * *std::max_element(H_null.begin(), H_null.end()) /
      *std::min_element(h_null.begin(), h_null.end());
  double delta_h = 0;
  LOOP_START(coords.lowerBound(start.key()), coords.lowerBound(end.value()),
             it);
  delta_h = std::max(pair.first * *it + pair.second - h.at(*it), delta_h);
  LOOP_END;
  //  if (delta_h > max_H0_h0) {
  //    auto phi_null = _mirror_coef();
  //    ostream << _attentuationPlane(phi_null, delta_h);
  //  } else {
  //    ostream << _attentuationPlane(1, delta_h);
  //  }
}

bool Opened::_isTangent(double a, double b) const {
  LOOP_START(coords.begin(), coords.end(), it);
  if (a * it.key() + b <= it.value()) return false;
  LOOP_END;
  return true;
}

double Opened::_attentuationPlane(double phi_null, double delta_h) {
  return -10 *
         log10(1 + qPow(phi_null, 2) -
               2 * qPow(phi_null, 2) * qCos(M_PI / 3 * delta_h * delta_h));
}  // Конец реализации расчета затухания на открытом интервале

// Составляющая расчета. Реализация расчета затухания на полуоткрытом интервале

bool SemiOpened::exec() {
  auto shad = _shadingObstacle();  ///< Координаты затеняющего препятствия
  double l_null_length =  ///< Длина участка отражения
      lNull(data->param.h_null[shad.first], k(shad.first));
  double a = obstacleSphereRadius(l_null_length, _deltaY(shad.first));
  //  if (a >= qSqrt(data->constant.area_length * data->constant.lambda * 0.5
  //  *
  //                 (0.5 / 3))) {
  //    _sphereApproximation(shad.first, a);
  //  } else {
  //    _wedgeApproximation(shad.first);
  //  }
  if (!_data) return false;
  return true;
}

double SemiOpened::_sphereApproximation(double x, double obst_sph_radius) {
  double kk = k(x);  ///< Относительная координата точки
  double mu =        ///< Параметр рельефа местности
      _areaReliefParameter(kk, data->param.H_null[x], obst_sph_radius);
  return _attentuationPSph(mu);
}

double SemiOpened::_wedgeApproximation(double x) {
  double kk = k(x);  ///< Относительная координата точки
  double nu =  ///<  Параметр клиновидного препятствия
      _nuWedg(data->param.H[x], kk);
  return _attentuationPWedg(nu);
}

double SemiOpened::_deltaY(double sh_ind) {
  QVector<int> intersect;  ///< Индексы точек пересчения высотного профиля и
                           ///< зоны Френеля

  LOOP_START(data->param.H_null.begin(), data->param.H_null.end() - 1, it);
  auto a =  ///< Величина высотного профиля
      it.value();
  auto b =  ///< Ордината нижней точки зоны Френеля с определенным индексом
      data->param.los.first * sh_ind + data->param.los.second - it.value();
  // Если a и b равны
  if (qAbs(b - a) <= 0.5) intersect.push_back(it.key());
  LOOP_END;
  if (intersect.size() == 1 || !intersect.size()) {
    return 0;
  }
  auto left =  ///< Первая точка пересечения
      *intersect.begin();
  auto right =  ///< Вторая точка пересечения
      *std::prev(intersect.end());
  if (right == left)
    right = *std::lower_bound(intersect.begin(), intersect.end(), right);
  auto [c, d] =  ///< Коэффициенты уравнения прямой
      strLineEquation(left, coords[left], right, coords[right]);
  return abs(coords[sh_ind] - c * sh_ind - d);
}  // namespace Land

QPair<double, double> SemiOpened::_shadingObstacle(void) const {
  double min_y = data->param.H[data->param.coords.b_x()];
  QMap<double, double>::iterator i = data->param.H.begin();
  LOOP_START(data->param.H.begin(), data->param.H.end(), it);
  i = (it.value() < min_y) ? it : i;
  LOOP_END;
  return {i.key(), i.value()};
}

double SemiOpened::_areaReliefParameter(double k, double H0,
                                        double obstacleShereRadius) {
  return pow(qPow(data->constant.area_length, 2) * k * k * ((1 - k) * (1 - k)) /
                 (obstacleShereRadius * H0),
             1.0 / 3);
}

double SemiOpened::_reliefParFuncSph(double mu) { return 4 + 10 / (mu - 0.1); }

double SemiOpened::_attentuationPSph(double mu) {
  return 6 + 16.4 / (mu * (1 + 0.8 * mu));
}

double SemiOpened::_nuWedg(double H, double k) {
  return -H * qSqrt(2 / (data->constant.lambda *
                         (data->constant.area_length / 2) * 2 * k * (1 - k)));
}

double SemiOpened::_attentuationPWedg(double nu) {
  return 6.9 + 20 * log10(qSqrt(pow(nu - 0.1, 2) + 1) + nu - 0.1);
}

double SemiOpened::_t(double l, double s) {
  return l / s;
}  // Конец реализации расчета затухания на полуоткрытом интервале

// Составляющая расчета. Реализация расчета затухания на закрытом интервале

bool Closed::exec() {
  Peaks l = _countPeaks();
  auto param = _reliefTangentStraightLines(l);
  ostream << _atten(param) << '\n';
  if (!_data) return false;
  return true;
}

auto Closed::_countPeaks() -> Peaks {
  Peaks v;
  double p;
  bool inside = 0;
  LOOP_START(coords.begin(), coords.end(), it);
  if (it.value() >=
          (data->param.los.first * it.key() + data->param.los.second) &&
      inside == 0) {
    p = it.key();
    inside = 1;
  } else if (it.value() <=
                 (data->param.los.first * it.key() + data->param.los.second) &&
             inside == 1) {
    inside = 0;
    v.push_back(qMakePair(p, it.key()));
  }
  LOOP_END;
  _approx(v);  // Аппроксимация нескольких участков одним
  return v;
}  // namespace Calc

void Closed::_approx(Peaks &v) {
  auto coord = coords.toMap();

  LOOP_START(v.begin(), v.end() - 1, it);
  auto r1 =  ///< Расстояние до вершины первого препятствия
      std::distance(coord.begin(),
                    std::max_element(std::next(coord.begin(), it->first),
                                     std::next(coord.begin(), it->second),
                                     [](const std::pair<int, int> &p1,
                                        const std::pair<int, int> &p2) {
                                       return p1.second < p2.second;
                                     }));
  auto r2 =  ///< Расстояние до вершины второго препятствия
      std::distance(
          coord.begin(),
          std::max_element(
              std::next(coord.begin(), std::next(it)->first),
              std::next(coord.begin(), std::next(it)->second),
              [](const std::pair<int, int> &p1, const std::pair<int, int> &p2) {
                return p1.second < p2.second;
              }));
  // Условие аппроксимации
  if (log10(M_PI - qAsin(qSqrt(data->constant.area_length * (r2 - r1) /
                               (r2 * (data->constant.area_length - r1))))) >
      0.408) {
    std::next(it)->first = it->first;
    v.erase(it);
  }
  LOOP_END;
}

double Closed::_reliefTangentStraightLines(const Peaks &p) {
  QPair<double, double> left,
      right;  ///< Координаты высшей точки правого и левого препятствия
  Peaks peaks;

  // Находим для каждого препятствия координаты его высшей точки
  for (auto it : p) {
    double max_x = -1, max_y = -1;
    for (auto i = it.first; i <= it.second; ++i) {
      if (max_y < coords[i]) max_y = coords[i], max_x = i;
    }
    peaks.push_back({max_x, max_y});
  }

  Peaks::iterator p_it = peaks.begin();
  LOOP_START(p.begin(), p.end(), it);
  left = (it == p.begin())
             ? qMakePair(data->tower.sender.first, data->tower.sender.second)
             : *(it - 1);
  right = (it == p.end() - 1) ? qMakePair(data->tower.reciever.first,
                                          data->tower.reciever.second)
                              : *(it + 1);
  _tangent(*it, left, right);
  p_it = std::next(p_it);
  LOOP_END;
}

double Closed::_tangent(const QPair<double, double> &p,
                        const QPair<double, double> &left,
                        const QPair<double, double> &right) {
  QPair<double, double> line_l, line_r;

  // Поиск касательной со стороны левого препятствия
  for (auto it = (coords.lowerBound(left.first) + 1); it != coords.end();
       it = std::next(it)) {
    // Если итератор вышел за пределы словаря
    if ((coords.end() - 1).key() - it.key() < 0) {
      estream << "Index out of interval range. Terminating\n";
      exit(0);
    }
    line_l = strLineEquation(left.first, left.second, it.key(), it.value());
    if (_isTangent(line_l.first, line_l.second, it.key(),
                   (coords.end() - 1).key())) {
      break;
    }
  }
  // Поиск касательной со стороны правого препятствия
  for (auto it = (coords.lowerBound(right.first) - 1); it != coords.begin();
       it = std::prev(it)) {
    line_r = strLineEquation(right.first, right.second, it.key(), it.value());
    if (_isTangent(line_r.first, line_r.second, (coords.begin()).key(),
                   it.key())) {
      break;
    }
  }
  auto poi = _pointOfIntersection(line_l, line_r);
  double h =
      poi.second - data->param.los.first * poi.first - data->param.los.second;
  double d1 = qSqrt(qPow(poi.second - left.second, 2) +
                    qPow(poi.first - left.first, 2));
  double d2 = qSqrt(qPow(poi.second - right.second, 2) +
                    qPow(poi.first - right.first, 2));
  return _diffractionParam(h, d1, d2);
}

QPair<double, double> Closed::_pointOfIntersection(QPair<double, double> l,
                                                   QPair<double, double> r) {
  double x = (r.second - l.second) / (l.first - r.first);
  double y = l.first * x + l.second;
  return {x, y};
}

double Closed::_diffractionParam(double h, double d1, double d2) {
  return h * qSqrt(2 * (d1 + d2) / (data->constant.lambda * d1 * d2));
}

double Closed::_atten(double param) {
  if (param <= -.7)
    return 0;
  else
    return 6.9 + 20 * log10(qSqrt(qPow((param - .1), 2) + 1) + param - .1);
}

}  // namespace Land
}  // namespace Atten

Core::Core(QCustomPlot *cp) {
  _data = QSharedPointer<Data>::create();
  _cp = cp;
  _main = Main::Item::Ptr::create(_data, _cp);
}

bool Core::exec() { return _main->exec(); }

}  // namespace Calc

}  // namespace NRrls
