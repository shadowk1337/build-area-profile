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

 private:
  QSharedPointer<NRrls::Calc::Data> data = _data.toStrongRef();
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
  QSharedPointer<NRrls::Calc::Data> data = _data.toStrongRef();
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
  QSharedPointer<NRrls::Calc::Data> data = _data.toStrongRef();

  /**
   * Функция построения уравнения прямой y = $a * x + $b
   * @param x - абсцисса первой точки
   * @param y - ордината первой точки
   * @param xx - абсцисса второй точки
   * @param yy - ордината второй точки
   * @return Пара{$a, $b}
   */
  std::pair<double, double> strLineEquation(double x, double y, double xx,
                                            double yy) const;
  int int_start = 0;  ///< Индекс начальной точки интервала
  int int_end = data->param.count - 1;  ///< Индекс конечной точки интервала
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
   * Функция нахождения наивысшей точки пересечения высотного профиля и отрезка,
   * соединяющего приемник и точку, зеркальную передатчику относительно
   * высотного профиля
   * @return Координаты наивысшей точки пересечения
   */
  std::pair<double, double> findPointOfIntersection(void);

  /**
   * Функция аппроксимации плоскостью
   * @param start - Индекс начальной точки участка отражения
   * @param end - Индекс конечной точки участка отражения
   */
  void planeApproximation(int start, int end);

  /**
   * Функция аппроксимации сферой
   * @param start - Индекс начальной точки участка отражения
   * @param end - Индекс конечной точки участка отражения
   */
  void sphereApproximation(int start, int end);

  /**
   * Функция сравнения с критерием Рэлея
   * @param start - Индекс начальной точки участка отражения
   * @param end - Индекс конечной точки участка отражения
   */
  void rayleighAndGroundCriteria(int start, int end);

  /**
   * Функция
   * @param piv
   * @param limit
   * @return
   */
  double findEl(int piv, int limit);

  /**
   * @brief
   * @param start - Индекс начальной точки участка отражения
   * @param end - Индекс конечной точки участка отражения
   * @return
   */
  std::pair<int, int> lineOfSightCoords(int start, int end);

 private:
  double attentuationPlane(double);
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
   * @param idx - индекс рассматриваемой точки
   * @param obst_sph_radius - радиус препятствия
   * @return Затухание на интервале
   */
  double sphereApproximation(int idx, double obst_sph_radius);

  /**
   * Функция аппроксимации клином
   * @param idx - индекс рассматриваемой точки
   * @return Затухание на интервале
   */
  double wedgeApproximation(int idx);

  /**
   * Функция нахождения высоты хорды
   * @param sh_height - высота затеняющего препятствия
   * @return Величина высоты хорды
   */
  double deltaY(int sh_ind);

  /**
   * Функция нахождения затеняющего препятствия
   * @return Пара .first - индекс препятствия .second - величина минимального
   * просвета
   */
  std::pair<int, double> shadingObstacle(void) const;

 private:
  /**
   * Функция нахождения параметра рельефа местности
   * @param k - относительная координата
   * @param H0 - критический просвет
   * @param obstacleShereRadius - радиус сферы препятствия
   * @return Величина параметра рельефа местности
   */
  inline double areaReliefParameter(double k, double H0,
                                    double obstacleShereRadius);

  /**
   * Функция нахождения функции параметра рельефа
   * @param mu - параметр рельефа местности
   * @return Величина функции параметра рельефа
   */
  inline double reliefParFuncSph(double mu);

  /**
   * Функция нахождения затухания
   * @param mu - параметр рельефа местности
   * @return Величина затухания
   */
  inline double attentuationPSph(double mu);

  /**
   * Функция нахождения параметра клиновидного препятствия
   * @param H - реальный просвет
   * @param k - относительная координата
   * @return Величина параметра клиновидного препятствия
   */
  inline double nuWedg(double H, double k);

  /**
   * Функция нахождения затухания
   * @param nu - параметр клиновидного препятствия
   * @return Величина затухания
   */
  inline double attentuationPWedg(double nu);

  /**
   * Функция нахождения относительной протяженности вершины препятствия
   * @param l - реальная протяеженность вершины
   * @param s - масштаб расстояний
   * @return Величина относительной протяженности вершины препятствия
   */
  inline double t(double l, double s);
};

/**
 * Составляющая расчета. Расчет затухания на закрытом интервале
 */
class Closed : public Land::Item {
 public:
  QSHDEF(Closed);
  typedef std::vector<std::pair<int, int>> Coords;
  Closed(const Data::WeakPtr &data) : Land::Item(data) {}

 public:
  bool exec() override;

 private:
  /**
   * Функция подсчета препятствий
   * @return Количество препятствий
   */
  const Coords countPeaks(void);

  /**
   * Функция аппроксимации одним эквивалентом
   * @param v - индексы затеняющих препятствий
   */
  void approx(std::vector<std::pair<int, int>> &v);

  /**
   * @brief reliefTangentStraightLines
   */
  void reliefTangentStraightLines(int start, int end);

  /**
   * @brief findMinHeight
   * @return
   */
  std::pair<int, double> findMinHeight(double, double, int, int);

  /**
   * @brief findLongestInterval
   * @return
   */
  int findLongestInterval(void) const;

  /**
   * @brief findlNull
   * @param p
   * @return
   */
  double findlNull(std::pair<int, double> p);

 private:
  /**
   * @brief isTangent
   * @return
   */
  inline bool isTangent(double, double, int, int) const;

  /**
   * @brief distanceSquare
   * @return
   */
  inline double distanceSquare(double);

  /**
   * @brief relativeDistances
   * @return
   */
  inline double relativeDistances(double, double);

  /**
   * @brief relativePoint
   * @return
   */
  inline double relativePoint(double, int);
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
      ostream << "Hello";
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
    estream << "Couldn't open heights file\n";
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
    estream << "File is empty";
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

std::pair<double, double> Atten::Land::Item::strLineEquation(double x, double y,
                                                             double xx,
                                                             double yy) const {
  double a = (yy - y);
  double b = (xx - x);
  return {a / b, y - (x * a / b)};
}

// Составляющая расчета. Реализация расчета затухания на открытом интервале

bool Atten::Land::Opened::exec() {
  const auto coords = findPointOfIntersection();
  double l_null_length =  ///< Длина участка отражения
      lNull(data->param.h_null.at(coords.first),
            k(coords.first * data->param.diff));
  int in_int_start =  ///< Индекс начальной точки участка отражения
      std::max(int_start, static_cast<int>(coords.first -
                                           l_null_length / data->param.diff));
  int in_int_end =  ///< Индекс конечной точки участка отражения
      std::min(int_end, static_cast<int>(coords.first +
                                         l_null_length / data->param.diff));
  // Если длина участка отражения меньше 1/4 длины всего интервала
  if (l_null_length <= 0.25 * data->constant.area_length)
    planeApproximation(in_int_start, in_int_end);
  else
    sphereApproximation(in_int_start, in_int_end);
  if (!_data) return false;
  return true;
}

std::pair<double, double> Atten::Land::Opened::findPointOfIntersection(void) {
  double opposite_y_coord =  ///< Ордината точки, зеркальной к передатчику
                             ///< относительно высотного профиля
      2 * data->param.heights.at(int_start) - data->tower.sender.second;
  auto [a, b] =
      strLineEquation(int_start, opposite_y_coord, int_end * data->param.diff,
                      data->tower.reciever.second);
  double intersection_index = -1;  ///< Индекс наивысшей точки пересечения
  double max_height = 0;  ///< Ордината наивысшей точки пересечения
  // Поиск точки пересечения, если их несколько, то берется
  // наивысшая точка
  for (auto it = int_start; it < int_end; ++it) {
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
  // Если точка пересечения не найдена
  if (intersection_index == -1) {
    estream << "Bad opened interval. Terminating.";
    exit(EXIT_FAILURE);
  }
  return {intersection_index, max_height};
}

void Atten::Land::Opened::planeApproximation(int start, int end) {
  rayleighAndGroundCriteria(start, end);
}

void Atten::Land::Opened::sphereApproximation(int start, int end) {
  auto h = data->param.heights;
  auto min_height =  ///< Значение минимальной высоты на интервале
      *std::min_element(h.begin() + start, h.begin() + end);
  auto max_height =  ///< Значение максимальной высоты на интервале
      *std::max_element(h.begin() + start, h.begin() + end);
  auto delta_y =  ///< Высота хорды между параболой и горизонталью
      qAbs(max_height - min_height);
  if (max_height == min_height) return;
  //  ostream << obstacleSphereRadius(
  //      (int_end - int_start) * data->intervals_difference, delta_y);
}

void Atten::Land::Opened::rayleighAndGroundCriteria(int start, int end) {
  double k, b, denominator, y, delta_h;
  auto h = data->param.heights;
  auto [a, z] = lineOfSightCoords(start, end);
  denominator = (z - a);
  k = (double(h.at(z) - h.at(a)) / denominator);
  b = h.at(a) - (a * (h.at(z) - h.at(a))) / denominator;
  auto max_H0_h0 =
      *std::max_element(data->param.HNull_hNull_div.begin() + start,
                        data->param.HNull_hNull_div.begin() + end);
  for (auto i = int_start; i <= int_end; ++i) {
    y = k * i + b;
    delta_h = abs(y - h.at(i));
    //        ostream << delta_h << 0.75 * max_H0_h0;
    //    if (delta_h < 0.75 * max_H0_h0) {
    //      ostream << i << ": Гладкая";
    //      ostream << attentuationPlane(delta_h);
    //    } else
    //      ostream << '-';
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

std::pair<int, int> Atten::Land::Opened::lineOfSightCoords(int start, int end) {
  int pivot = (int_end + int_start) / 2;
  double max_first = int_start, max_last = int_end;
  max_last = findEl(pivot, int_end);
  max_first = findEl(pivot, int_start);
  return {max_first, max_last};
}

double Atten::Land::Opened::attentuationPlane(double delta_h) {
  return -10 * log10(2 - 2 * qCos(M_PI / 3 * delta_h * delta_h));
}  // Конец реализации расчета затухания на открытом интервале

// Составляющая расчета. Реализация расчета затухания на полуоткрытом интервале

bool Atten::Land::SemiOpened::exec() {
  auto shad = shadingObstacle();  ///< Координаты затеняющего препятствия
  double l_null_length =  ///< Длина участка отражения
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
  if (!_data) return false;
  return true;
}

double Atten::Land::SemiOpened::sphereApproximation(int idx,
                                                    double obst_sph_radius) {
  double kk = k(idx * data->param.diff);  ///< Относительная координата точки
  double mu =  ///< Параметр рельефа местности
      areaReliefParameter(kk, data->param.H_null.at(idx), obst_sph_radius);
  return attentuationPSph(mu);
}

double Atten::Land::SemiOpened::wedgeApproximation(int idx) {
  double kk = k(idx * data->param.diff);  ///< Относительная координата точки
  double nu =  ///<  Параметр клиновидного препятствия
      nuWedg(data->param.H.at(idx), kk);
  return attentuationPWedg(nu);
}

double Atten::Land::SemiOpened::deltaY(int sh_ind) {
  std::vector<int>
      intersect;  ///< Индексы точек пересчения высотного профиля и зоны Френеля
  for (size_t ind = 0; ind + 1 <= data->param.count; ++ind) {
    auto a =  ///< Величина высотного профиля
        data->param.heights.at(ind);
    auto b =  ///< Ордината нижней точки зоны Френеля с определенным индексом
        data->param.los_heights.at(ind) - data->param.H_null.at(ind);
    // Если a и b равны
    if (qAbs(b - a) <= 0.5) intersect.push_back(ind);
  }
  if (intersect.size() == 1 || !intersect.size()) {
    return 0;
  }
  auto left =  ///< Первая точка пересечения
      *intersect.begin();
  auto right =  ///< Вторая точка пересечения
      *std::lower_bound(intersect.rbegin(), intersect.rend(), int_end,
                        [](int a, int b) { return a > b; });
  if (right == left)
    right = *std::lower_bound(intersect.begin(), intersect.end(), right);
  auto [c, d] =  ///< Коэффициенты уравнения прямой
      strLineEquation(left * data->param.diff, data->param.heights.at(left),
                      right * data->param.diff, data->param.heights.at(right));
  return abs(data->param.heights.at(sh_ind) - c * sh_ind * data->param.diff -
             d);
}

std::pair<int, double> Atten::Land::SemiOpened::shadingObstacle(void) const {
  auto it = std::min_element(data->param.H.begin(), data->param.H.end());
  double dist = std::distance(data->param.H.begin(), it);
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
}  // Конец реализации расчета затухания на полуоткрытом интервале

// Составляющая расчета. Реализация расчета затухания на закрытом интервале

bool Atten::Land::Closed::exec() {
  Coords v = countPeaks();
  for (auto it : v) reliefTangentStraightLines(it.first, it.second);
  if (!_data) return false;
  return true;
}

const Atten::Land::Closed::Coords Atten::Land::Closed::countPeaks(void) {
  Coords v;
  int p;
  int i = 0;
  bool inside = 0;
  for (auto it : data->param.los_heights) {
    if (it <= data->param.heights.at(i) && inside == 0) {
      p = i;
      inside = 1;
    } else if (it >= data->param.heights.at(i) && inside == 1) {
      inside = 0;
      v.push_back({p, i});
    }
    i++;
  }
  approx(v);
  return v;
}

void Atten::Land::Closed::approx(Coords &v) {
  for (auto i = v.begin(); i != v.end() - 1; ++i) {
    auto r1 =  ///< Расстояние до вершины первого препятствия
        std::distance(
            data->param.heights.begin(),
            std::max_element(data->param.heights.begin() + i->first,
                             data->param.heights.begin() + i->second));
    auto r2 =  ///< Расстояние до вершины второго препятствия
        std::distance(
            data->param.heights.begin(),
            std::max_element(data->param.heights.begin() + next(i)->first,
                             data->param.heights.begin() + next(i)->second));
    // Условие аппроксимации
    if (log10(M_PI - qAsin(qSqrt(data->constant.area_length * (r2 - r1) /
                                 (r2 * (data->constant.area_length - r1))))) >
        0.408) {
      next(i)->first = i->first;
      v.erase(i);
    }
  }
}

void Atten::Land::Closed::reliefTangentStraightLines(int start, int end) {
  std::pair<int, double> min_height_send, min_height_rec;
  double ind_send, ind_rec;  ///< Индексы точек касания

  for (auto i = start; i <= end; ++i) {
    if (i == start || i == end) continue;
    auto [a_sender, b_sender] =  ///< Уравнение прямой для передатчика
        strLineEquation(data->tower.sender.first, data->tower.sender.second,
                        i * data->param.diff, data->param.heights.at(i));
    auto [a_reciever, b_reciever] =  ///< Уравнение прямой для приемника
        strLineEquation(data->tower.reciever.first, data->tower.reciever.second,
                        i * data->param.diff, data->param.heights.at(i));
    if (isTangent(a_sender, b_sender, start, end)) {
      ind_send = i;
      min_height_send = findMinHeight(a_sender, b_sender, int_start, int_end);
      QCPItemLine *line = new QCPItemLine(_cp);
      line->start->setCoords(data->tower.sender.first,
                             data->tower.sender.second);
      line->end->setCoords(i * data->param.diff,
                           a_sender * i * data->param.diff + b_sender);
    }
    if (isTangent(a_reciever, b_reciever, start, end)) {
      ind_rec = i;
      min_height_rec =
          findMinHeight(a_reciever, b_reciever, start, end);
      QCPItemLine *line = new QCPItemLine(_cp);
      line->start->setCoords(data->tower.reciever.first,
                             data->tower.reciever.second);
      line->end->setCoords(i * data->param.diff,
                           a_sender * i * data->param.diff + b_sender);
    }
  }
  auto [x, p] = (min_height_send.second < min_height_rec.second)
                    ? std::make_pair(ind_send, min_height_send)
                    : std::make_pair(ind_rec, min_height_rec);
  double delta_y = qAbs(data->param.heights.at(x) - p.second);
  double a = obstacleSphereRadius(findlNull(p) * data->param.diff, delta_y);
  double s = distanceSquare(a);
  //  ostream << s;
  //  ostream << relativeDistances(s, min_height_send.first *
  //  data->param.diff); ostream << relativeDistances(
  //      s, qAbs(data->param.heights.size() - min_height_rec.first - 1));
}

bool Atten::Land::Closed::isTangent(double a, double b, int int_start,
                                    int int_end) const {
  for (int j = int_start; j <= int_end; ++j) {
    if (a * j * data->param.diff + b < data->param.heights.at(j)) return false;
  }
  return true;
}

std::pair<int, double> Atten::Land::Closed::findMinHeight(double a, double b,
                                                          int start, int end) {
  auto min_height =
      std::max_element(data->param.heights.begin(), data->param.heights.end());
  int height_index = 0;
  QVector<double> x, y;
  for (int i = start, k = 0; i <= end; ++i, ++k) {
    x.push_back(i * data->param.diff);
    y.push_back(a * i * data->param.diff + b - qAbs(data->param.H_null.at(i)));
    ostream << x.at(k) << " " << y.at(k) << '\n';

    if (y.at(k) < data->param.heights.at(i) + 1 &&
        y.at(k) > data->param.heights.at(i) - 1) {
      *min_height = std::min(*min_height, data->param.heights.at(i));
      height_index = i;
    }
  }
  return {height_index, *min_height};
}

double Atten::Land::Closed::findlNull(
    std::pair<int, double> p) {  // TODO: нужен больший обзор
  auto it = std::find_if(data->param.heights.begin() + int_start,
                         data->param.heights.begin() + int_end, [=](int val) {
                           return (val + 1 > p.second) && (val - 1 < p.second);
                         });
  return qAbs(std::distance(it, data->param.heights.begin() + p.first));
}

double Atten::Land::Closed::distanceSquare(double obstSphRadius) {
  return qPow((obstSphRadius * obstSphRadius * data->constant.lambda) / M_PI,
              0.33333);
}

double Atten::Land::Closed::relativeDistances(double s, double r) {
  return s / r;
}

double Atten::Land::Closed::relativePoint(double a, int i) {
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
