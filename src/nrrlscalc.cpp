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
  QCustomPlot *cp = data->mainWindow->customplot_1;
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
   * @param a       - y = a * x + b
   * @param b       - y = a * x + b
   * @param start   - начальный индекс рассматриваемого интервала
   * @param end     - конечный индекс рассматриваемого интервала
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
  QPair<int, int> _findPointOfIntersection(void);

  /**
   * Функция аппроксимации плоскостью
   * @param start   - Индекс начальной точки участка отражения
   * @param end     - Индекс конечной точки участка отражения
   */
  void _planeApproximation(QPair<int, int> intersec);

  /**
   * Функция аппроксимации сферой
   * @param start   - Индекс начальной точки участка отражения
   * @param end     - Индекс конечной точки участка отражения
   */
  void _sphereApproximation(QPair<int, int> intersec);

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
   * Функция нахождения затеняющего препятствия
   * @return Пара .first
   *                - индекс препятствия .second - величина минимального
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

namespace Diagram {

class Item : public Calc::Item {
 public:
  QSHDEF(Item);
  Item(const Data::WeakPtr &data) : Calc::Item(data) {}

 protected:
  QSharedPointer<Calc::Data> data = _data.toStrongRef();
  QCustomPlot *cp1 = data->mainWindow->customplot_2;
  QCustomPlot *cp2 = data->mainWindow->customplot_3;

 public:
  virtual bool exec() override;
};

class Axes : public Diagram::Item {
 public:
  QSHDEF(Axes);
  Axes(const Data::WeakPtr &data) : Item(data) {}

 protected:
  QSharedPointer<Calc::Data> data = _data.toStrongRef();

 public:
  bool exec() override;
};

class Setup : public Diagram::Item {
 public:
  QSHDEF(Setup);
  Setup(const Data::WeakPtr &data) : Item(data) {}

 protected:
  QSharedPointer<Calc::Data> data = _data.toStrongRef();

 public:
  bool exec() override;

 private:
  void fstStation(const QVector<double> &x);
  void sndStation(const QVector<double> &x);
};

}  // namespace Diagram

namespace Etc {

void setGraph(QCustomPlot *cp, int it, const QVector<double> &x,
              const QVector<double> &y, QPen pen, QString name = "Graph") {
  cp->addGraph();
  cp->graph(it)->setData(x, y, true);
  cp->graph(it)->setPen(pen);
  cp->graph(it)->setName(name);
}

}  // namespace Etc

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
  _items = {
      qMakePair(Fill::Item::Ptr::create(_data), QVector<int>{0}),
      qMakePair(Profile::Item::Ptr::create(_data), QVector<int>{0}),
      qMakePair(Interval::Item::Ptr::create(_data), QVector<int>{0}),
      qMakePair(Atten::Land::Item::Ptr::create(_data), QVector<int>{0, 1}),
      qMakePair(Atten::Free::Item::Ptr::create(_data), QVector<int>{0, 1}),
      qMakePair(Atten::Air::Item::Ptr::create(_data), QVector<int>{0, 1}),
      qMakePair(Atten::Acceptable::Item::Ptr::create(_data), QVector<int>{0}),
      qMakePair(Median::Item::Ptr::create(_data), QVector<int>{0, 1}),
      qMakePair(Diagram::Item::Ptr::create(_data), QVector<int>{1})};
}

bool Item::exec() {
  auto data_m = _data.toStrongRef()->mainWindow;
  for (auto &item : _items) {
    if (item.second.contains(data_m->stackwidget->currentIndex()))
      if (!item.first->exec()) return false;
  }
  data_m->customplot_1->replot();
  data_m->customplot_2->replot();
  data_m->customplot_3->replot();
  return true;
}

}  // namespace Main

bool Profile::Item::exec() {
  return (
      Axes::Ptr::create(_data)->exec() && Earth::Ptr::create(_data)->exec() &&
      Fresnel::Ptr::create(_data)->exec() && Los::Ptr::create(_data)->exec());
}

bool Diagram::Item::exec() {
  return (Axes::Ptr::create(_data)->exec() &&
          Setup::Ptr::create(_data)->exec());
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
  for (auto it = coords.lowerBound(start); it != coords.lowerBound(end);
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
  //  cp->setInteractions(QCP::iSelectPlottables | QCP::iRangeDrag |
  //                      QCP::iRangeZoom);

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
  QVector<double> x, y, h;
  x.reserve(data->param.count);
  y.reserve(data->param.count);
  h.reserve(data->param.count);

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
  Etc::setGraph(cp, 0, x, y, pen, QObject::tr("Уровень моря"));

  coords.y(h);

  for (int i = 0; i < h.size() && i < y.size(); ++i) h[i] += y[i];

  pen = QPen(QColor("#137ea8"), 2);
  Etc::setGraph(cp, 1, x, h, pen, QObject::tr("Высотный профиль"));

  // Обновление словаря координат
  int i = 0;
  for (auto jt = coords.begin(); jt != coords.end(); ++jt, ++i)
    jt.value() = h[i];

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
  LOOP_START(coords.begin(), coords.end(), it);
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
  LOOP_START(coords.begin(), coords.end(), i);
  y.push_back((-data->param.H_null[i.key()]) + data->param.los.first * i.key() +
              data->param.los.second);
  LOOP_END;

  QPen pen(Qt::red, 2);
  Etc::setGraph(cp, 2, x, y, pen, QObject::tr("Зона Френеля"));

  auto it = x.begin();
  LOOP_START(y.begin(), y.end(), i);
  *i += 2 * data->param.H_null[*it];
  it = (it != x.end()) ? std::next(it) : std::prev(x.end());
  LOOP_END;

  Etc::setGraph(cp, 3, x, y, pen);

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
  Etc::setGraph(cp, 4, x, y, pen, QObject::tr("Линия прямой видимости"));

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
      data->mainWindow->label_intervalType->setText(QObject::tr("Полуоткрыт."));
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
  (l_null_length <= 0.25 * data->constant.area_length)
      ? _planeApproximation(point)
      : _sphereApproximation(point);
  if (!_data) return false;
  return true;
}

QPair<int, int> Opened::_findPointOfIntersection(void) {
  double oppositendY_coord =  ///< Ордината точки, зеркальной к передатчику
                              ///< относительно высотного профиля
      coords.startY() - data->tower.f.second;
  auto pair = strLineEquation(coords.startX(), oppositendY_coord, coords.endX(),
                              data->tower.s.second + coords.endY());
  //  QCPItemLine *line = new QCPItemLine(data->mainWindow->customplot_1);
  //  line->start->setCoords(coords.startX(), oppositendY_coord);
  //  line->end->setCoords(coords.endX(),
  //                       data->tower.reciever.second + coords.endY());
  double inters_x;  ///< Индекс наивысшей точки пересечения высотного
  ///< профиля и прямой, проведенной из точки приемника к точке, зеркальной
  ///< передатчику
  double inters_y;  ///< Ордината точки пересечения
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
  return {inters_x, inters_y};
}

void Opened::_planeApproximation(QPair<int, int> intersec) {
  ostream << 's';
  double delta_r = qPow(data->param.H[intersec.first], 2) /
                   (2 * data->constant.area_length * k(intersec.first) *
                    (1 - k(intersec.first)));
}

void Opened::_sphereApproximation(QPair<int, int> intersec) {
  auto coord = coords.toMap();
  auto h_max = std::max_element(
      coord.begin(), coord.end(),
      [](const std::pair<double, double> &lhs,
         const std::pair<double, double> &rhs) { return rhs > lhs; });
  QPair<double, double> parallel_line = {
      data->param.los.first, data->param.los.second -
                                 data->param.H[intersec.first] -
                                 data->param.H_null[intersec.first]};
  //  for () {

  //  }
}

double Opened::_attentuationPlane(double phi_null, double delta_h) {
  return -10 *
         log10(1 + qPow(phi_null, 2) -
               2 * qPow(phi_null, 2) * qCos(M_PI / 3 * delta_h * delta_h));
}  // Конец реализации расчета затухания на открытом интервале

// Составляющая расчета. Реализация расчета затухания на полуоткрытом интервале

bool SemiOpened::exec() {
  auto shad = _shadingObstacle();  ///< Координаты затеняющего препятствия
  double atten = data->wp = _atten(_tangent(shad));
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
  auto p = coords.lowerBound(min_H->first);
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
  double atten = data->wp = _atten(_reliefTangentStraightLines(l));
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
  for (auto it = (coords.lowerBound(left.first) + 1); it != coords.end();
       ++it) {
    line_l = strLineEquation(left.first, left.second, it.key(), it.value());
    if (_isTangent(line_l.first, line_l.second, it.key(), coords.endX())) break;
  }

  // Поиск касательной со стороны правого препятствия
  for (auto it = (coords.lowerBound(right.first) - 1); it != coords.begin();
       --it) {
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
  data->wa = data->constant.area_length / 1000.0 *
             ((1 - (data->constant.temperature - 15) * .01) * gamma_oxygen +
              (1 - (data->constant.temperature - 15) * .06) * gamma_water);

  if (!_data) return false;
  return true;
}

bool Acceptable::Item::exec() {
  double to_uv = qPow(10.0, data->spec.s.first / 20.0);

  double to_dbvt = 10 * log10(qPow(to_uv * 1e-6, 2) / 50);
  data->mainWindow->label_attenValue->setText(
      QString::number(qAbs(to_dbvt - data->p.first + data->ws + data->wa)));
  data->mainWindow->label_attenValue_2->setText(QString::number(data->wp));
  ostream << qAbs(to_dbvt - data->p.first + data->ws + data->wa) << ' ';

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

  if (!_data) return false;
  return true;
}

}  // namespace Median

namespace Diagram {

bool Axes::exec() {
  cp1->yAxis->setVisible(true);
  cp1->yAxis2->setVisible(true);
  cp1->yAxis->setTickLength(0);
  cp1->yAxis2->setTickLength(0);
  cp1->yAxis->setSubTickLength(0);
  cp1->yAxis2->setSubTickLength(0);

  cp2->yAxis->setVisible(true);
  cp2->yAxis2->setVisible(true);
  cp2->yAxis->setTickLength(0);
  cp2->yAxis2->setTickLength(0);
  cp2->yAxis->setSubTickLength(0);
  cp2->yAxis2->setSubTickLength(0);

  if (!_data) return false;
  return true;
}

bool Setup::exec() {
  QVector<double> x(6);

  x[0] = 0, x[2] = x[1] = .1 * data->constant.area_length;
  x[4] = x[3] = x[2] + data->constant.area_length;
  x[5] = x[4] + .1 * data->constant.area_length;

  fstStation(x);
  sndStation(x);

  if (!_data) return false;
  return true;
}

void Setup::fstStation(const QVector<double> &x) {
  QVector<double> y(6);

  y[0] = data->spec.p.first;
  y[1] = y[0] - data->tower.wf.first;
  y[2] = y[1] + data->tower.c.first;
  y[3] = y[2] - data->ws;
  y[4] = y[3] + data->tower.c.second;
  y[5] = data->p.first + data->wa + data->wp;

  cp1->clearGraphs();
  cp1->addGraph();
  cp1->graph(0)->setPen(QPen(Qt::blue, 2));
  cp1->graph(0)->addData(x, y);

  y[3] -= data->wp, y[4] = y[3] + data->tower.c.second, y[5] -= data->wp;
  cp1->addGraph();
  cp1->graph(1)->setPen(QPen(Qt::red, 2));
  cp1->graph(1)->addData(x, y);

  y[3] -= data->wa, y[4] = y[3] + data->tower.c.second, y[5] = data->p.first;
  cp1->addGraph();
  cp1->graph(2)->setPen(QPen(Qt::black, 2));
  cp1->graph(2)->setPen(QPen(Qt::DashLine));
  cp1->graph(2)->addData(x, y);
  cp1->rescaleAxes();
  cp1->yAxis2->rescale();
}

void Setup::sndStation(const QVector<double> &x) {
  QVector<double> y(6);

  y[0] = data->spec.p.second;
  y[1] = y[0] - data->tower.wf.second;
  y[2] = y[1] + data->tower.c.second;
  y[3] = y[2] - data->ws;
  y[4] = y[3] + data->tower.c.first;
  y[5] = data->p.second + data->wa + data->wp;

  cp2->clearGraphs();
  cp2->addGraph();
  cp2->graph(0)->setPen(QPen(Qt::blue, 2));
  cp2->graph(0)->addData(x, y);

  y[3] -= data->wp, y[4] = y[3] + data->tower.c.first, y[5] -= data->wp;
  cp2->addGraph();
  cp2->graph(1)->setPen(QPen(Qt::red, 2));
  cp2->graph(1)->addData(x, y);

  y[3] -= data->wa, y[4] = y[3] + data->tower.c.first, y[5] = data->p.second;
  cp2->addGraph();
  cp2->graph(2)->setPen(QPen(Qt::black, 2));
  cp2->graph(2)->setPen(QPen(Qt::DashLine));
  cp2->graph(2)->addData(x, y);
  cp2->rescaleAxes();
  cp2->yAxis2->rescale();
}

}  // namespace Diagram

Core::Core(Ui::NRrlsMainWindow *m, const QString &filename) {
  _data = QSharedPointer<Data>::create();
  _data->mainWindow = m;
  _data->filename = filename;
  _main = Main::Item::Ptr::create(_data);
}

bool Core::exec() { return _main->exec(); }

void Core::setFreq(double f) {
  _data->spec.f = f;
  _data->constant.lambda = (double)3e+8 / (f * 1e+6);
}

void Core::setFHeight(double h) { _data->tower.f.second = h; }

void Core::setSHeight(double h) { _data->tower.s.second = h; }

double Core::setFCoef(double c) {
  _data->tower.c.first = c;
  return c;
}

double Core::setSCoef(double c) {
  _data->tower.c.second = c;
  return c;
}

void Core::setFFeedAtten(double f) { _data->tower.wf.first = f; }

void Core::setSFeedAtten(double f) { _data->tower.wf.second = f; }

double Core::setFPower(double p) {
  _data->spec.p.first = p;
  return p;
}

double Core::setSPower(double p) {
  _data->spec.p.second = p;
  return p;
}

double Core::setFSensetivity(double s) {
  _data->spec.s.first = s;
  return s;
}

double Core::setSSensetivity(double s) {
  _data->spec.s.second = s;
  return s;
}

void Core::setGradient(double g) { _data->constant.g_standard = g * 1e-08; }

void Core::setTemperature(double t) { _data->constant.temperature = t; }

}  // namespace Calc

}  // namespace NRrls
