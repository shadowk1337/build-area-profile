#include "nrrlscalc.h"

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
  Item(Calc::Data::WeakPtr &data) : Calc::Item(data) {}

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
  Item(const Calc::Data::WeakPtr &data) : Calc::Item(data) {}

 public:
  virtual bool exec() override;

 protected:
  QSharedPointer<Calc::Data> data = _data.toStrongRef();
  QCustomPlot *cp = data->mainWindow->customplot;
  decltype(data->param.coords) coords = data->param.coords;
};

/**
 * Составляющая расчета. Задание осей графика
 */
class Axes : public Profile::Item {
 public:
  QSHDEF(Axes);
  Axes(const Calc::Data::WeakPtr &data) : Profile::Item(data) {}

 public:
  bool exec() override;
};

/**
 * Составляющая расчета. Построение земной поверхности
 */
class Earth : public Profile::Item {
 public:
  QSHDEF(Earth);
  Earth(const Calc::Data::WeakPtr &data) : Profile::Item(data) {}

 public:
  virtual bool exec() override;

 private:
  void paramFill();
};

/**
 * Составляющая расчета. Построение зоны Френеля
 */
class Fresnel : public Profile::Item {
 public:
  QSHDEF(Fresnel);
  Fresnel(const Calc::Data::WeakPtr &data) : Profile::Item(data) {}

 public:
  bool exec() override;
};

/**
 * Составляющая расчета. Построение ЛПВ
 */
class Los : public Profile::Item {
 public:
  QSHDEF(Los);
  Los(const Calc::Data::WeakPtr &data) : Profile::Item(data) {}

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
  /**
   * Функция проверки касательности прямой к высотному профилю
   * @param a         - y = a * x + b
   * @param b         - y = a * x + b
   * @param start     - начальный индекс рассматриваемого интервала
   * @param end       - конечный индекс рассматриваемого интервала
   * @return Будет ли прямая касательной
   */
  bool _isTangent(double a, double b, double start, double end) const;

  QSharedPointer<Calc::Data> data = _data.toStrongRef();
  decltype(data->param.coords) coords = data->param.coords;
};

/**
 * Составляющая расчета. Расчет затухания на открытом интервале
 */
class Opened : public Land::Item {
 public:
  QSHDEF(Opened);
  using iter = QMap<double, double>::const_iterator;
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
  QPair<int, int> _findPointOfIntersection(void);

  /**
   * Функция аппроксимации плоскостью
   * @param start     - Индекс начальной точки участка отражения
   * @param end       - Индекс конечной точки участка отражения
   */
  double _planeApproximation(QPair<int, int> intersec);

  /**
   * Функция аппроксимации сферой
   * @param begin     - Индекс начальной точки участка отражения
   * @param end       - Индекс конечной точки участка отражения
   */
  double _sphereApproximation(iter begin, iter end);

  double _relief(iter begin, iter end);

 private:
  /**
   * Функция расчета затухания на открытом интервале
   * @param phi_null  - коэффициент отражения
   * @param p         - относительный просвет в точке отражения
   */
  inline double _atten(double phi_null, double p);
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
   * Функция нахождения затеняющего препятствия
   * @return Пара .first  - индекс препятствия
   *              .second - величина минимального
   * просвета
   */
  QPair<double, double> _shadingObstacle(void) const;

  double _tangent(const QPair<double, double> &p);

 private:
  inline QPair<double, double> _pointOfIntersection(
      QPair<double, double> l, QPair<double, double> r) const;

  inline double _diffractionParam(double h, double d1, double d2) const;

  inline double _atten(double param) const;
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
  bool exec() override;

 private:
  QSharedPointer<Calc::Data> data = _data.toStrongRef();
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
  bool exec() override;

 private:
  QSharedPointer<Calc::Data> data = _data.toStrongRef();
};

}  // namespace Air

namespace Acceptable {

/**
 * Составляющая расчета. Расчет допустимой велиины затухания на рельефе
 */
class Item : public Calc::Item {
 public:
  QSHDEF(Item);
  Item(const Data::WeakPtr &data) : Calc::Item(data) {}

 public:
  bool exec() override;

 private:
  QSharedPointer<Calc::Data> data = _data.toStrongRef();
};

}  // namespace Acceptable

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
  bool exec() override;
};

}  // namespace Median

double Item::k(double R) const {
  return R / (_data.toStrongRef()->constant.area_length);
}

double Item::lNull(double h0, double k) const {
  return ((_data.toStrongRef()->constant.area_length) * qSqrt(1 + h0 * h0)) /
         (1 + (h0 * h0) / (4 * k * (1 - k)));
}

double Item::HNull(double i) const {
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

namespace Main {

Item::Item(const Data::WeakPtr &data) : Master::Item(data) {
  _items = {Fill::Item::Ptr::create(_data),
            Profile::Item::Ptr::create(_data),
            Interval::Item::Ptr::create(_data),
            Atten::Land::Item::Ptr::create(_data),
            Atten::Free::Item::Ptr::create(_data),
            Atten::Air::Item::Ptr::create(_data),
            Median::Item::Ptr::create(_data),
            Atten::Acceptable::Item::Ptr::create(_data)};
}

bool Item::exec() {
  auto data_m = _data.toStrongRef()->mainWindow;
  data_m->customplot->clearGraphs();
  for (auto &item : _items) {
    if (!item->exec()) return false;
  }
  data_m->customplot->replot();
  return true;
}

}  // namespace Main

bool Profile::Item::exec() {
  return (
      Axes::Ptr::create(_data)->exec() && Earth::Ptr::create(_data)->exec() &&
      Fresnel::Ptr::create(_data)->exec() && Los::Ptr::create(_data)->exec());
}

bool Atten::Land::Item::exec() {
  switch (data->interval_type) {
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

bool Atten::Land::Item::_isTangent(double a, double b, double start,
                                   double end) const {
  for (auto it = data->param.coordsAndEarth.lowerBound(start);
       it != data->param.coordsAndEarth.lowerBound(end);
       it += (end - start >= 0) ? 1 : -1) {
    if (a * it.key() + b < it.value()) return false;
  }
  return true;
}

namespace Fill {

bool Item::exec() {
  QFile file(data->filename);
  QTextStream in(&file);
  bool first = true;
  auto &coords = data->param.coords;
  size_t &count = data->param.count;
  count = 0;

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    estream << QString("Couldn't open file %1\n").arg(data->filename);
    return false;
  }
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
      estream << QString("File %1 doesn't contain table names\n")
                     .arg(data->filename);
      return false;
    }
    count++;
    line.replace(",", ".");
    data->param.coordsAndEarth[line.section(";", l, l).toDouble()] =
        coords[line.section(";", l, l).toDouble()] =
            line.section(";", h, h).toDouble();
  }
  file.close();
  if (!count) {
    estream << QString("File %1 is empty\n").arg(data->filename);
    return false;
  }
  paramFill();

  if (_data.isNull()) return false;
  return true;
}

void Item::paramFill(void) {
  auto &coords = data->param.coords;
  data->tower.f.first = coords.startX();
  data->tower.s.first = coords.endX();
  data->constant.area_length = coords.endX() - coords.startX();
  data->param.los = strLineEquation(
      data->tower.f.first, data->tower.f.second + coords.startY(),
      data->tower.s.first, data->tower.s.second + coords.endY());
}

}  // namespace Fill

namespace Profile {

bool Axes::exec() {
  cp->xAxis->setVisible(1);
  cp->xAxis2->setVisible(1);
  cp->yAxis->setVisible(1);
  cp->yAxis2->setVisible(1);

  QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
  //    cp->setInteractions(QCP::iSelectPlottables | QCP::iRangeDrag |
  //                        QCP::iRangeZoom);

  for (double i = 0; i < data->constant.area_length; i += 1000) {
    QString str = QString::number(static_cast<int>(i) / 1000);
    textTicker->addTicks({{i, str}, {i + 1000, ""}});
  }

  QVector<double> heights;
  coords.y(heights);

  cp->yAxis->setSubTickLength(0);
  cp->yAxis->setTickLengthIn(0);
  cp->yAxis->setTickLengthOut(3);
  cp->yAxis->grid()->setVisible(false);
  cp->yAxis2->setSubTickLength(0);
  cp->yAxis2->setTickLengthIn(0);
  cp->yAxis2->setTickLengthOut(3);

  cp->xAxis->setTicker(textTicker);
  cp->xAxis->setTickLengthIn(0);
  cp->xAxis->setTickLengthOut(3);
  cp->xAxis->setRange(0, data->constant.area_length);
  cp->xAxis2->setTickLength(0);
  cp->xAxis2->setTickLabels(0);
  cp->xAxis2->setRange(0, data->constant.area_length);

  if (_data.isNull()) return false;
  return true;
}

bool Earth::exec() {
  QVector<double> x, y;
  x.reserve(data->param.count);
  y.reserve(data->param.count);

  bool first = true;
  double move_graph_up_value;
  double half = data->constant.area_length / 2;
  double equivalent_radius =
      data->constant.radius /
      (1 + data->constant.g_standard * data->constant.radius / 2);

  coords.x(x);
  auto it = x.begin();

  for (double i = -half; it != x.end(); it = std::next(it), i = -half + *it) {
    if (first) {
      first = false;
      move_graph_up_value = i * i / (2 * equivalent_radius);
      y.push_back(0);
    } else {
      y.push_back(-(i * i / (2 * equivalent_radius)) + move_graph_up_value);
    }
  }

  QPen pen(QColor("#014506"), 2);
  data->gr->draw(x, y, pen, QObject::tr("Уровень моря"));

  auto jt = data->param.coordsAndEarth.begin();
  for (int i = 0; jt != data->param.coordsAndEarth.end() && i < y.size();
       ++i, ++jt)
    jt.value() += y[i];

  QVector<double> h;
  h.reserve(data->param.count);
  foreach (auto i, data->param.coordsAndEarth.values())
    h.push_back(i);

  pen = QPen(QColor("#137ea8"), 2);
  data->gr->draw(x, h, pen, QObject::tr("Высотный профиль"));

  double h_max = *std::max_element(h.begin(), h.end());
  double max_graph_height =
      std::max(h_max, std::max(coords.startY() + data->tower.f.second,
                               coords.endY() + data->tower.s.second));
  double window_add_height = .2 * max_graph_height;
  double y_max =  ///< Высота видимости графика
      max_graph_height + window_add_height;

  cp->yAxis->setRange(0, y_max, Qt::AlignLeft);
  cp->yAxis2->setRange(0, y_max);

  x.clear(), y.clear();

  paramFill();

  if (!_data) return false;
  return true;
}

void Earth::paramFill() {
  LOOP_START(data->param.coordsAndEarth.begin(),
             data->param.coordsAndEarth.end(), it);
  data->param.H[it.key()] =
      data->param.los.first * it.key() + data->param.los.second - it.value();
  data->param.H_null[it.key()] = HNull(it.key());
  data->param.h_null[it.key()] =
      (data->param.H[it.key()] / data->param.H_null[it.key()]);
  LOOP_END;
  data->param.H_null[data->param.count - 1] = 0;
}

bool Fresnel::exec() {
  QVector<double> x, y;
  x.reserve(data->param.count);
  y.reserve(data->param.count);

  coords.x(x);
  LOOP_START(data->param.coordsAndEarth.begin(),
             data->param.coordsAndEarth.end(), i);
  y.push_back((-data->param.H_null[i.key()]) + data->param.los.first * i.key() +
              data->param.los.second);
  LOOP_END;

  QPen pen(Qt::red, 2);
  data->gr->draw(x, y, pen, QObject::tr("Зона Френеля"));

  auto it = x.begin();
  LOOP_START(y.begin(), y.end(), i);
  *i += 2 * data->param.H_null[*it];
  it = (it != x.end()) ? std::next(it) : std::prev(x.end());
  LOOP_END;

  data->gr->draw(x, y, pen);

  x.clear(), y.clear();

  if (!_data) return false;
  return true;
}

bool Los::exec() {
  QVector<double> x, y;
  x.reserve(data->param.count);
  y.reserve(data->param.count);

  coords.x(x);
  for (auto it : qAsConst(x))
    y.push_back(data->param.los.first * it + data->param.los.second);

  QPen pen(QColor("#d6ba06"), 2);
  data->gr->draw(x, y, pen, QObject::tr("Линия прямой видимости"));

  x.clear(), y.clear();

  if (!_data) return false;
  return true;
}

}  // namespace Profile

bool Interval::Item::exec() {
  data->interval_type = 0;

  LOOP_START(data->param.coords.begin(), data->param.coords.end(), it);
  auto i = it.key();
  if (data->param.H[i] >= data->param.H_null[i])
    data->interval_type = std::max(data->interval_type, 1);
  else if (data->param.H_null[i] > data->param.H[i] && data->param.H[i] > 0)
    data->interval_type = std::max(data->interval_type, 2);
  else if (0 > data->param.H[i])
    data->interval_type = 3;
  LOOP_END;

  switch (data->interval_type) {
    case (1):
      data->mainWindow->label_intervalType->setText(QObject::tr("Открытый"));
      break;
    case (2):
      data->mainWindow->label_intervalType->setText(
          QObject::tr("Полуоткрытый"));
      break;
    case (3):
      data->mainWindow->label_intervalType->setText(QObject::tr("Закрытый"));
      break;
  }
  if (!_data) return false;
  return true;
}

namespace Atten {
namespace Land {

// Составляющая расчета. Реализация расчета затухания на открытом интервале

bool Opened::exec() {
  const auto point = _findPointOfIntersection();
  double l_null_length =  ///< Длина участка отражения
      lNull(data->param.h_null[point.first], k(point.first));

  // Если длина участка отражения <= 1/4 длины всего интервала
  double delta_r =  ///< Разность хода между прямым и отраженным лучами
      (l_null_length <= 0.25 * data->constant.area_length)
          ? _planeApproximation(point)
          : _sphereApproximation(
                coords.lowerBound(point.first - l_null_length / 2),
                coords.lowerBound(point.first +
                                  l_null_length / 2));  // TODO: доделать

  double p =  ///< Относительный просвет в точке отражения
      qSqrt(6 * delta_r * data->constant.lambda);

  data->wp = _atten(
      _relief(
          data->param.coordsAndEarth.lowerBound(point.first - l_null_length),
          data->param.coordsAndEarth.lowerBound(point.first + l_null_length)),
      p);

  if (!_data) return false;
  return true;
}

QPair<int, int> Opened::_findPointOfIntersection(void) {
  double oppositendY_coord =  ///< Ордината точки, зеркальной к передатчику
                              ///< относительно высотного профиля
      coords.startY() - data->tower.f.second;

  auto pair =  ///< Линия, проведенная к зеркальной точке
      strLineEquation(coords.startX(), oppositendY_coord, coords.endX(),
                      data->tower.s.second + coords.endY());

  double inters_x;  ///< Индекс наивысшей точки пересечения высотного
  ///< профиля и прямой, проведенной из точки приемника к точке, зеркальной
  ///< передатчику
  double inters_y;  ///< Ордината точки пересечения

  // Поиск точки пересечения высотного профиля и линии, проведенной к зеркальной
  // точке. Если точек несколько, то берется последняя в цикле точка
  LOOP_START(std::next(data->param.coordsAndEarth.begin()),
             std::prev(data->param.coordsAndEarth.end()), it);
  auto y_coord = pair.first * it.key() + pair.second;
  if ((y_coord >= (it - 1).value() && y_coord <= (it + 1).value()) ||
      (y_coord <= (it - 1).value() && y_coord >= (it + 1).value())) {
    inters_y = y_coord;
    inters_x = it.key();
  }
  LOOP_END;
  return {inters_x, inters_y};
}

double Opened::_planeApproximation(QPair<int, int> intersec) {
  return qPow(data->param.H[intersec.first], 2) /
         (2 * data->constant.area_length * k(intersec.first) *
          (1 - k(intersec.first)));
}

double Opened::_sphereApproximation(iter begin, iter end) {
  auto min_h = std::min_element(data->param.H.lowerBound(begin.key()),
                                data->param.H.lowerBound(end.key()));

  return data->param.h_null.lowerBound(min_h.key()).value();
}

double Opened::_relief(iter begin, iter end) {
  auto line =
      strLineEquation(begin.key(), begin.value(), end.key(), end.value());

  int res = 0;
  double delta_h_max = 0;
  for (auto it = begin; it != end; ++it) {
    double delta_h = (line.second * it.key() - line.first);
    double h_max =
        0.75 * (data->param.H_null[it.key()] - data->param.h_null[it.key()]);
    if (delta_h <= h_max)
      continue;
    else if (delta_h < data->param.H_null[it.key()])
      res = std::min(res, 1);
    else
      res = 2;
    delta_h_max = std::max(delta_h_max, delta_h);
  }
  if (qAbs(delta_h_max) - .1 <= 0) res = 3;

  return res == 0 ? 1
                  : data->constant.reflection_coef.lowerBound(
                        data->constant.lambda) ==
                            data->constant.reflection_coef.end()
                        ? data->constant.reflection_coef.last()[res - 1]
                        : data->constant.reflection_coef
                              .lowerBound(data->constant.lambda)
                              .value()[res - 1];
}

double Opened::_atten(double phi_null, double p) {
  return -10 * log10(1 + qPow(phi_null, 2) -
                     2 * qPow(phi_null, 2) * qCos(2 * M_PI * (p * p) / 3));
}  // Конец реализации расчета затухания на открытом интервале

// Составляющая расчета. Реализация расчета затухания на полуоткрытом интервале

bool SemiOpened::exec() {
  auto shad = _shadingObstacle();  ///< Координаты затеняющего препятствия
  data->wp = _atten(_tangent(shad));

  if (!_data) return false;
  return true;
}

QPair<double, double> SemiOpened::_shadingObstacle(void) const {
  auto H = data->param.H.toStdMap();
  auto min_H = std::min_element(H.begin(), H.end(),
                                [&](const std::pair<double, double> &lhs,
                                    const std::pair<double, double> &rhs) {
                                  return rhs.second > lhs.second;
                                });
  auto p = data->param.coordsAndEarth.lowerBound(min_H->first);

  return {p.key(), p.value()};
}

double SemiOpened::_tangent(const QPair<double, double> &p) {
  auto line_l =
      strLineEquation(coords.startX(), coords.startY(), p.first, p.second);
  auto line_r =
      strLineEquation(coords.endX(), coords.endY(), p.first, p.second);

  auto poi = _pointOfIntersection(line_l, line_r);
  double h =  ///< Возвышение препятствия над ЛПВ
      poi.second - data->param.los.first * poi.first - data->param.los.second;
  double d1 =  ///< Расстояние от левого конца трассы интервала до препятствия
      qSqrt(qPow(poi.second - coords.startY(), 2) +
            qPow(poi.first - coords.startX(), 2));
  double d2 =  ///< Расстояние от правого конца трассы интервала до препятствия
      qSqrt(qPow(poi.second - coords.endY(), 2) +
            qPow(poi.first - coords.endX(), 2));
  return _diffractionParam(h, d1, d2);
}

QPair<double, double> SemiOpened::_pointOfIntersection(
    QPair<double, double> l, QPair<double, double> r) const {
  double x = (r.second - l.second) / (l.first - r.first);
  double y = l.first * x + l.second;
  return {x, y};
}

double SemiOpened::_diffractionParam(double h, double d1, double d2) const {
  return h * qSqrt(2 * (d1 + d2) / (data->constant.lambda * d1 * d2));
}

double SemiOpened::_atten(double param) const {
  if (param <= -.7)
    return 0;
  else
    return 6.9 + 20 * log10(qSqrt(qPow((param - .1), 2) + 1) + param - .1);
}

// Конец реализации расчета затухания на полуоткрытом интервале

// Составляющая расчета. Реализация расчета затухания на закрытом интервале

bool Closed::exec() {
  Peaks l = _countPeaks();
  data->wp = _atten(_reliefTangentStraightLines(l));

  if (!_data) return false;
  return true;
}

auto Closed::_countPeaks() -> Peaks {
  Peaks v;
  double p;
  bool inside = 0;
  LOOP_START(data->param.coordsAndEarth.begin(),
             data->param.coordsAndEarth.end(), it);
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
    (it + 1)->first = it->first;
    v.erase(it);
  }
  LOOP_END;
}

double Closed::_reliefTangentStraightLines(const Peaks &p) {
  QPair<double, double> left,  ///< Координаты высшей точки левого препятствия
      right;  ///< Координаты высшей точки правого препятствия
  Peaks peaks;
  double diffraction_param = 0;

  // Находим для каждого препятствия координаты его высшей точки
  for (auto &it : qAsConst(p)) {
    double max_x = -1, max_y = -1;
    for (auto i = it.first; i <= it.second; ++i) {
      if (max_y < coords[i]) max_y = coords[i], max_x = i;
    }
    peaks.push_back({max_x, max_y});
  }

  LOOP_START(peaks.begin(), peaks.end(), it);
  left = (it == peaks.begin())
             ? qMakePair(data->tower.f.first,
                         data->tower.f.second + data->param.coords.startY())
             : *(it - 1);
  right = (it == peaks.end() - 1)
              ? qMakePair(data->tower.s.first,
                          data->tower.s.second + data->param.coords.endY())
              : *(it + 1);
  diffraction_param += _tangent(*it, left, right);
  LOOP_END;
  return diffraction_param;
}

double Closed::_tangent(const QPair<double, double> &p,
                        const QPair<double, double> &left,
                        const QPair<double, double> &right) {
  QPair<double, double> line_l, line_r;

  // Поиск касательной со стороны левого препятствия
  for (auto it = (data->param.coordsAndEarth.lowerBound(left.first) + 1);
       it != data->param.coordsAndEarth.end(); ++it) {
    line_l = strLineEquation(left.first, left.second, it.key(), it.value());
    if (_isTangent(line_l.first, line_l.second, it.key(), coords.endX())) break;
  }

  // Поиск касательной со стороны правого препятствия
  for (auto it = (data->param.coordsAndEarth.lowerBound(right.first) - 1);
       it != data->param.coordsAndEarth.begin(); --it) {
    line_r = strLineEquation(right.first, right.second, it.key(), it.value());
    if (_isTangent(line_r.first, line_r.second, coords.startX(), it.key()))
      break;
  }

  auto poi = _pointOfIntersection(line_l, line_r);
  double h =  ///< Возвышение препятствия над ЛПВ
      poi.second - data->param.los.first * poi.first - data->param.los.second;
  double d1 =  ///< Расстояние от левого конца трассы интервала до препятствия
      qSqrt(qPow(poi.second - left.second, 2) +
            qPow(poi.first - left.first, 2));
  double d2 =  ///< Расстояние от правого конца трассы интервала до препятствия
      qSqrt(qPow(poi.second - right.second, 2) +
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

bool Free::Item::exec() {
  data->ws = 122 + 20 * log10((data->constant.area_length / 1e+3) /
                              (data->constant.lambda * 1e+2));

  if (!_data) return false;
  return true;
}

bool Air::Item::exec() {
  double f = data->spec.f / 1000.0;
  double gamma_oxygen =
      (6.09 / (f * f + .227) + 7.19e-3 + 4.81 / (qPow(f - 57, 2) + 1.5)) * f *
      f * 1e-3;
  double gamma_water =
      (.05 + 3.6 / (qPow(f - 22.2, 2) + 8.5) + 21e-4 * 7.5 +
       10.6 / (qPow(f - 188.3, 2) + 9) + 8.9 / (qPow(f - 325.4, 2) + 26.3)) *
      f * f * 7.5 * 1e-4;
  data->wa = (data->constant.area_length / 1000.0) *
             ((1 - (data->constant.temperature - 15) * .01) * gamma_oxygen +
              (1 - (data->constant.temperature - 15) * .06) * gamma_water);

  if (!_data) return false;
  return true;
}

bool Acceptable::Item::exec() {
  double to_uv =
      qPow(10.0, ((abs(data->spec.s.first) > abs(data->spec.s.second))
                      ? data->spec.s.first
                      : data->spec.s.second) /
                     20.0);

  double to_dbvt = 10 * log10(qPow(to_uv * 1e-6, 2) / 50);

  double at = qAbs(
      to_dbvt - (((abs(data->p.first) < abs(data->p.second)) ? data->p.first
                                                             : data->p.first) +
                 data->wp + data->wa));

  data->mainWindow->label_attenValue->setText(QString::number(at));
  data->mainWindow->label_attenValue_2->setText(QString::number(data->wp));
  data->mainWindow->label_attenValue_3->setText(QString::number(data->ws));
  data->mainWindow->label_attenValue_4->setText(QString::number(data->wa));
  data->mainWindow->label_concValue->setText(at < data->wp
                                                 ? QObject::tr("Связи не будет")
                                                 : QObject::tr("Связь будет"));
  if (!_data) return false;
  return true;
}

}  // namespace Atten

namespace Median {

bool Item::exec() {
  auto data = _data.toStrongRef();

  data->p.first = data->spec.p.first - data->tower.wf.first +
                  data->tower.c.first - data->wp - data->ws - data->wa +
                  data->tower.c.second - data->tower.wf.second;

  data->p.second = data->spec.p.second - data->tower.wf.second +
                   data->tower.c.second - data->wp - data->ws - data->wa +
                   data->tower.c.first - data->tower.wf.first;

  data->log_p.first = C(data->spec.p.first) - C(data->tower.wf.first) +
                      C(data->tower.c.first) - C(data->wp) - C(data->ws) -
                      C(data->wa) + C(data->tower.c.second) -
                      C(data->tower.wf.second);

  data->log_p.second = C(data->spec.p.second) - C(data->tower.wf.second) +
                       C(data->tower.c.second) - C(data->wp) - C(data->ws) -
                       C(data->wa) + C(data->tower.c.first) -
                       C(data->tower.wf.first);

  if (!_data) return false;
  return true;
}

}  // namespace Median

Core::Core(Ui::NRrlsMainWindow *m, const QString &filename) {
  data = QSharedPointer<Data>::create();
  data->mainWindow = m;
  data->gr = QSharedPointer<GraphPainter>::create(m->customplot);
  data->filename = filename;
  _main = Main::Item::Ptr::create(data);
}

bool Core::exec() { return _main->exec(); }

void Core::setFreq(double f) {
  data->spec.f = f;
  data->constant.lambda = (double)3e+8 / (f * 1e+6);
}

double Core::coordX(double c) {
  return data->param.coordsAndEarth.lowerBound(c).key();
}

double Core::coordY(double c) {
  return data->param.coordsAndEarth.lowerBound(c).value();
}

double Core::xRange() {
  return data->mainWindow->customplot->xAxis->range().size();
}

double Core::yRange() {
  return data->mainWindow->customplot->yAxis->range().size();
}

}  // namespace Calc

}  // namespace NRrls
