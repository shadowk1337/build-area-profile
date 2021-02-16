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
  std::pair<double, double> _findPointOfIntersection(void);

  /**
   * Функция аппроксимации плоскостью
   * @param start   - Индекс начальной точки участка отражения
   * @param end     - Индекс конечной точки участка отражения
   */
  void _planeApproximation(double start, double end);

  /**
   * Функция аппроксимации сферой
   * @param start   - Индекс начальной точки участка отражения
   * @param end     - Индекс конечной точки участка отражения
   */
  void _sphereApproximation(double start, double end);

  /**
   * Функция сравнения с критерием Рэлея
   * @param start   - Индекс начальной точки участка отражения
   * @param end     - Индекс конечной точки участка отражения
   */
  void _rayleighAndGroundCriteria(double mid, double length);

 private:
  double _attentuationPlane(double phi_null, double delta_h);
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
   * @param idx     - индекс рассматриваемой точки
   * @param obst_sph_radius
   *                - радиус препятствия
   * @return Затухание на интервале
   */
  double _sphereApproximation(int idx, double obst_sph_radius);

  /**
   * Функция аппроксимации клином
   * @param idx     - индекс рассматриваемой точки
   * @return Затухание на интервале
   */
  double _wedgeApproximation(int idx);

  /**
   * Функция нахождения высоты хорды
   * @param sh_height
   *                - высота затеняющего препятствия
   * @return Величина высоты хорды
   */
  double _deltaY(int sh_ind);

  /**
   * Функция нахождения затеняющего препятствия
   * @return Пара .first
   *                - индекс препятствия .second - величина минимального
   * просвета
   */
  std::pair<int, double> _shadingObstacle(void) const;

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
  typedef QList<std::pair<double, double>> Peaks;
  Closed(const Data::WeakPtr &data) : Land::Item(data) {}

 public:
  bool exec() override;

 private:
  //  QVector<double> h;

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
  void _approx(QList<std::pair<double, double>> &v);

  /**
   * Функция нахождения прямых, касательных высотному профилю на отрезке [start,
   * end]
   * @param start   - индекс начальной точки
   * @param end     - индекс конечной точки
   */
  void _reliefTangentStraightLines(const Peaks &p);

  /**
   * Функция построения уравнений прямых и проверки касательности их к высотному
   * профилю
   * @param p       - кортеж, в который будут передаваться данные
   * @param start   - индекс входа ЛПВ в препятствие
   * @param end     - индекс выхода ЛПВ из препятствия
   * @param type    - передатчик/приемник
   */
  void _checkTangent(std::tuple<int, std::pair<int, double>> *p, int start,
                     int end, std::string type);

  /**
   * @brief findlNull
   * @param p
   * @return
   */
  double _findlNull(std::pair<int, double> p);

 private:
  /**
   * Функция проверки касательности прямой к высотному профилю
   * @param a       - y = a * x + b
   * @param b       - y = a * x + b
   * @param start   - начальный индекс рассматриваемого интервала
   * @param end     - конечный индекс рассматриваемого интервала
   * @return Будет ли прямая касательной
   */
  inline bool _isTangent(double a, double b, int int_start, int int_end) const;

  /**
   * @brief distanceSquare
   * @return
   */
  inline double _distanceSquare(double);

  /**
   * @brief relativeDistances
   * @return
   */
  inline double _relativeDistances(double, double);

  /**
   * @brief relativePoint
   * @return
   */
  inline double _relativePoint(double, int);
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

namespace Etc {

void setGraph(QCustomPlot *cp, const QVector<double> &x,
              const QVector<double> &y, QPen pen = QPen{}) {
  static int it = 0;
  cp->addGraph();
  cp->graph(it)->setData(x, y, true);
  cp->graph(it++)->setPen(pen);
}

}  // namespace Etc

double Item::k(double R) {
  return R / (_data.toStrongRef()->constant.area_length);
}

double Item::lNull(double h0, double k) {
  return ((_data.toStrongRef()->constant.area_length) * qSqrt(1 + h0 * h0)) /
         (1 + (h0 * h0) / (4 * k * (1 - k)));
}

double Item::HNull(int i) {
  auto data = _data.toStrongRef();
  return sqrt(data->constant.area_length * data->constant.lambda * k(i) *
              ((1 - k(i)) / 3));
}

double Item::obstacleSphereRadius(double l0, double delta_y) {
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
  QString filename = "heights3.csv";
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
  system("sed -i '/^*$/d' heights3.csv");
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
  y.push_back((i == coords.begin()) ? 0
                                    : (-data->param.H_null[i.key()]) +
                                          data->param.los.first * i.key() +
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
// Составляющая расчета. Реализация расчета затухания на открытом интервале

bool Atten::Land::Opened::exec() {
  const auto point = _findPointOfIntersection();
  double l_null_length =  ///< Длина участка отражения
      lNull(data->param.h_null[point.first], k(point.first));
  double start =  ///< Индекс начальной точки участка отражения
      std::max(calc_start.x(), point.first - l_null_length / 2);
  double end =  ///< Индекс конечной точки участка отражения
      std::min(std::prev(coords.end()).key(), point.first + l_null_length / 2);
  // Если длина участка отражения <= 1/4 длины всего интервала
  if (l_null_length <= 0.25 * data->constant.area_length)
    _planeApproximation((end - start) / 2, l_null_length);
  else
    _sphereApproximation(start, end);
  if (!_data) return false;
  return true;
}

std::pair<double, double> Atten::Land::Opened::_findPointOfIntersection(void) {
  double opposite_y_coord =  ///< Ордината точки, зеркальной к передатчику
                             ///< относительно высотного профиля
      2 * calc_start.y() - data->tower.sender.second;
  auto pair = strLineEquation(calc_start.x(), opposite_y_coord,
                              std::prev(coords.end()).key(),
                              data->tower.reciever.second);
  double inters_x = -1;  ///< Индекс наивысшей точки пересечения высотного
  ///< профиля и прямой, проведенной из точки приемника к точке, зеркальной
  ///< передатчику
  double inters_y = -1;  ///< Ордината точки пересечения
  // Поиск точки пересечения, если их несколько, то берется
  // последняя в цикле точка
  LOOP_START(std::next(coords.begin()), std::prev(coords.end()), it);
  auto y_coord = pair.first * it.key() + pair.second;
  if (it != coords.begin())
    if ((y_coord >= (it - 1).value() && y_coord <= (it + 1).value()) ||
        (y_coord <= (it - 1).value() && y_coord >= (it + 1).value())) {
      inters_y = y_coord;
      inters_x = it.key();
    }
  LOOP_END;
  // Если точка пересечения не найдена
  if (inters_x < 0 || inters_y < 0) {
    estream << "Bad opened interval. Terminating.\n";
    exit(EXIT_FAILURE);
  }
  return {inters_x, inters_y};
}

void Atten::Land::Opened::_planeApproximation(double mid, double length) {
  _rayleighAndGroundCriteria(mid, length);
}

void Atten::Land::Opened::_sphereApproximation(double start, double end) {
  ostream << 's';
  //  auto min_height = ///< Значение минимальной высоты на интервале
  //      std::min_element(coords.lowerBound(start), coords.lowerBound(end),
  //      [](const decltype(coords)::iterator lhs, const
  //      decltype(coords)::iterator rhs) {
  //        return lhs.second < rhs.second);
  //        ostream << &min_height;
  //  auto max_height = ///< Значение максимальной высоты на интервале
  //      *std::max_element(h.begin() + start, h.begin() + end);
  //  auto delta_y = ///< Высота хорды между параболой и горизонталью
  //      qAbs(max_height - min_height);
  //  if (max_height == min_height)
  //    return;
  //  ostream << obstacleSphereRadius(start - end, delta_y);
}

void Atten::Land::Opened::_rayleighAndGroundCriteria(double mid,
                                                     double length) {
  QVector<double> h, x;
  coords.y(h);
  std::map<double, double> coord = coords.toMap();
  std::map<double, double>::iterator temp;
  auto start =  ///< Начальная точка интервала отражения
      *std::lower_bound(coord.begin(), coord.end(), mid - length / 2,
                        [](const std::pair<double, double> &p, double value) {
                          return p.first < value;
                        });
  auto end =  ///< Конечная точка интервала отражения
      ((temp = std::lower_bound(
            coord.begin(), coord.end(), mid + length / 2,
            [](const std::pair<double, double> &p, double value) {
              return p.first < value;
            })) == coord.end())
          ? *std::prev(coord.end())
          : *temp;
  auto pair = strLineEquation(start.first, start.second, end.first, end.second);
  auto H_null = data->param.H_null.values();
  auto h_null = data->param.h_null.values();
  auto max_H0_h0 =  ///< Максимально допустимое значение неровности рельефа
      0.75 * *std::max_element(H_null.begin(), H_null.end()) /
      *std::min_element(h_null.begin(), h_null.end());
  int cond = 1;  ///< Условие неровности рельефа (1 - гладкая, 2 -
                 ///< малопересеч/среднепересеч)
  double delta_h = 0;
  LOOP_START(coords.lowerBound(start.first), coords.lowerBound(end.first), it);
  delta_h = std::max(pair.first * *it + pair.second - h.at(*it), delta_h);
  if (delta_h > max_H0_h0) cond = 2;
  LOOP_END;
  if (cond == 1) ostream << _attentuationPlane(1, delta_h);
  //  else
  //    ostream << _attentuationPlane()
}

double Atten::Land::Opened::_attentuationPlane(double phi_null,
                                               double delta_h) {
  return -10 *
         log10(1 + qPow(phi_null, 2) -
               2 * qPow(phi_null, 2) * qCos(M_PI / 3 * delta_h * delta_h));
}  // Конец реализации расчета затухания на открытом интервале

// Составляющая расчета. Реализация расчета затухания на полуоткрытом интервале
/*
bool Atten::Land::SemiOpened::exec() {

  auto shad = _shadingObstacle(); ///< Координаты затеняющего препятствия
  double l_null_length = ///< Длина участка отражения
      lNull(data->param.h_null.at(shad.first),
            k(shad.first * data->param.diff));
  double a = obstacleSphereRadius(l_null_length, deltaY(shad.first));
  int_end = a;
  if (a >= qSqrt(data->constant.area_length * data->constant.lambda * 0.5 *
                 (0.5 / 3))) {
    sphereApproximation(shad.first, a);
  } else {
    wedgeApproximation(shad.first);
  }
  if (!_data)
    return false;
  return true;
}

double Atten::Land::SemiOpened::_sphereApproximation(int idx,
                                                     double obst_sph_radius) {
  double kk = k(idx * data->param.diff); ///<
  Относительная координата точки double mu = ///< Параметр рельефа местности
      _areaReliefParameter(kk, data->param.H_null.at(idx), obst_sph_radius);
  return _attentuationPSph(mu);
}

double Atten::Land::SemiOpened::_wedgeApproximation(int idx) {
  double kk = k(idx * data->param.diff); ///< Относительная координата
  точки double nu = ///<  Параметр клиновидного препятствия
      _nuWedg(data->param.H.at(idx), kk);
  return _attentuationPWedg(nu);
}

double Atten::Land::SemiOpened::_deltaY(int sh_ind) {
  QVector<int> intersect; ///< Индексы точек пересчения высотного профиля
  и зоны Френеля for (size_t ind = 0; ind + 1 <= data->param.count; ++ind) {
    auto a =
        ///< Величина высотного профиля h.at(ind); auto b = ///< Ордината
        нижней
            ///< точки
            зоны Френеля с определенным индексом data->param.los_heights.at(
                ind) -
        data->param.H_null.at(ind);
    // Если a и b равны
    if (qAbs(b - a) <= 0.5)
      intersect.push_back(ind);
  }
  if (intersect.size() == 1 || !intersect.size()) {
    return 0;
  }
  auto left = ///< Первая точка пересечения
      *intersect.begin();
  auto right = ///< Вторая точка пересечения
      *std::lower_bound(intersect.rbegin(), intersect.rend(), int_end,
                        [](int a, int b) { return a > b; });
  if (right == left)
    right = *std::lower_bound(intersect.begin(), intersect.end(), right);
  auto [c, d] = ///< Коэффициенты уравнения прямой
      strLineEquation(left * data->param.diff, h.at(left),
                      right * data->param.diff, h.at(right));
  return abs(h.at(sh_ind) - c * sh_ind * data->param.diff - d);
}

std::pair<int, double> Atten::Land::SemiOpened::shadingObstacle(void) const {
  auto it = std::min_element(data->param.H.begin(), data->param.H.end());
  double dist = std::distance(data->param.H.begin(), it);
  return {dist, *it};
}

double
Atten::Land::SemiOpened::_areaReliefParameter(double k, double H0,
                                              double obstacleShereRadius) {
  return pow(qPow(data->constant.area_length, 2) * k * k * ((1 - k) * (1 - k))
/ (obstacleShereRadius * H0), 1.0 / 3);
}

double Atten::Land::SemiOpened::reliefParFuncSph(double mu) {
  return 4 + 10 / (mu - 0.1);
}

double Atten::Land::SemiOpened::attentuationPSph(double mu) {
  return 6 + 16.4 / (mu * (1 + 0.8 * mu));
}

double Atten::Land::SemiOpened::nuWedg(double H, double k) {
  return -H * qSqrt(2 / (data->constant.lambda * data->constant.r_evaluated *
                         2 * k * (1 - k)));
}

double Atten::Land::SemiOpened::attentuationPWedg(double nu) {
  return 6.9 + 20 * log10(qSqrt(pow(nu - 0.1, 2) + 1) + nu - 0.1);
}

double Atten::Land::SemiOpened::t(double l, double s) {
  return l / s;
} // Конец реализации расчета затухания на полуоткрытом интервале
*/
// Составляющая расчета. Реализация расчета затухания на закрытом интервале

bool Atten::Land::Closed::exec() {
  Peaks l = _countPeaks();
  _reliefTangentStraightLines(it.first, it.second);
  if (!_data) return false;
  return true;
}

auto Atten::Land::Closed::_countPeaks() -> Peaks {
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
    v.push_back({p, it.key()});
  }
  LOOP_END;
  _approx(v);  // Аппроксимация нескольких участков одним
  return v;
}  // namespace Calc

void Atten::Land::Closed::_approx(QList<std::pair<double, double>> &v) {
  auto coord = coords.toMap();
  for (auto i = v.begin(); i != v.end() - 1; ++i) {
    auto r1 =  ///< Расстояние до вершины первого препятствия
        std::distance(coord.begin(),
                      std::max_element(std::next(coord.begin(), i->first),
                                       std::next(coord.begin(), i->second),
                                       [](const std::pair<int, int> &p1,
                                          const std::pair<int, int> &p2) {
                                         return p1.second < p2.second;
                                       }));
    auto r2 =  ///< Расстояние до вершины второго препятствия
        std::distance(
            coord.begin(),
            std::max_element(std::next(coord.begin(), std::next(i)->first),
                             std::next(coord.begin(), std::next(i)->second),
                             [](const std::pair<int, int> &p1,
                                const std::pair<int, int> &p2) {
                               return p1.second < p2.second;
                             }));
    // Условие аппроксимации
    if (log10(M_PI - qAsin(qSqrt(data->constant.area_length * (r2 - r1) /
                                 (r2 * (data->constant.area_length - r1))))) >
        0.408) {
      std::next(i)->first = i->first;
      v.erase(i);
    }
  }
}

void Atten::Land::Closed::_reliefTangentStraightLines(const Peaks &p) {
  QPair<double, double> left, right;
  LOOP_START(p.begin(), p.end(), it);
  left = (it == p.begin())
             ? qMakePair(data->tower.sender.first, data->tower.sender.second)
             : std::prev(it);
  LOOP_END;
  //  std::tuple<int, std::pair<int, double>> sender = {}, reciever = {};
  //  _checkTangent(&sender, start, end, "sender");
  //  _checkTangent(&reciever, start, end, "reciever");
}

void Atten::Land::Closed::_checkTangent(
    std::tuple<int, std::pair<int, double>> *p, int start, int end,
    std::string type) {
  std::pair<double, double> pa;

  for (auto i = start; i <= end; ++i) {
    if (i == start || i == end) continue;
    if (type == "sender")
      pa =  ///< Уравнение прямой
          strLineEquation(data->tower.sender.first, data->tower.sender.second,
                          i, h.at(i));
    else if (type == "reciever")
      pa =  ///< Уравнение прямой
          strLineEquation(data->tower.reciever.first,
                          data->tower.reciever.second, i * data->param.diff,
                          h.at(i));
    else
      return;
    if (_isTangent(pa.first, pa.second, start, end)) {
      // Если прямая является касательной к высотному профилю на отрезке
[start,
      // end] *p = std::make_tuple(i, );
      return;
    }
  }
}

bool Atten::Land::Closed::_isTangent(double a, double b, int start,
                                     int end) const {
  for (int j = start; j <= end; ++j) {
    if (a * j + b < h.at(j)) return false;
  }
  return true;
}

double Atten::Land::Closed::_findlNull(std::pair<int, double> p) {
  auto it = std::find_if(h.begin() + start, h.begin() + end, [=](int val) {
    return (val + 1 > p.second) && (val - 1 < p.second);
  });
  return qAbs(std::distance(it, h.begin() + p.first));
}

double Atten::Land::Closed::_distanceSquare(double obstSphRadius) {
  return qPow((obstSphRadius * obstSphRadius * data->constant.lambda) / M_PI,
              0.33333);
}

double Atten::Land::Closed::_relativeDistances(double s, double r) {
  return s / r;
}

double Atten::Land::Closed::_relativePoint(double a, int i) {
  auto z = k(i);
  return (a * data->param.H[i]) / (data->constant.area_length * z * (1 - z));
}  // Конец реализации расчета затухания на закрытом интервале

Core::Core(QCustomPlot *cp) {
  _data = QSharedPointer<Data>::create();
  _cp = cp;
  _main = Main::Item::Ptr::create(_data, _cp);
}

bool Core::exec() { return _main->exec(); }

}  // namespace Calc

}  // namespace NRrls

/*
  QCPItemLine *line = new QCPItemLine(_cp);
      line->start->setCoords(data->tower.sender.first,
                             data->tower.sender.second);
      line->end->setCoords(i * data->param.diff,
                           a_sender * i * data->param.diff + b_sender);
*/
