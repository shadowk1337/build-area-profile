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
  bool exec() override;
};

/**
 * Составляющая расчета. Построение высотного профиля
 */
class Curve : public Profile::Item {
 public:
  QSHDEF(Curve);
  Curve(const Calc::Data::WeakPtr &data) : Profile::Item(data) {}

 public:
  bool exec() override;
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

namespace Feeder {

/**
 * Составляющая расчета. Расчет затухания в фидере на передачу и прием
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

}  // namespace Feeder

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
  QCustomPlot *cp = data->mainWindow->customplot_2;

 public:
  bool exec() override;
};

class Axes : public Item {
 public:
  QSHDEF(Item);
  Axes(const Data::WeakPtr &data) : Item(data) {}

 protected:
  QSharedPointer<Calc::Data> data = _data.toStrongRef();

 public:
  bool exec() override;
};

}  // namespace Diagram

namespace Etc {

void setGraph(QCustomPlot *cp, int it, const QVector<double> &x,
              const QVector<double> &y, QPen pen) {
  cp->addGraph();
  cp->graph(it)->setData(x, y, true);
  cp->graph(it)->setPen(pen);
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
      qMakePair(Fill::Item::Ptr::create(_data), QString("Fill")),
      qMakePair(Profile::Item::Ptr::create(_data), QString("Profile")),
      qMakePair(Interval::Item::Ptr::create(_data), QString("Interval")),
      qMakePair(Atten::Land::Item::Ptr::create(_data), QString("Attentuation")),
      qMakePair(Atten::Free::Item::Ptr::create(_data), QString("Free")),
      qMakePair(Atten::Air::Item::Ptr::create(_data), QString("Air")),
      //      qMakePair(Atten::Feeder::Item::Ptr::create(_data),
      //      QString("Feeder")), qMakePair(Median::Item::Ptr::create(_data),
      //      QString("Median")), qMakePair(Diagram::Item::Ptr::create(_data),
      //      QString("Diagram"))
  };
}

bool Item::exec() {
  auto data_m = _data.toStrongRef()->mainWindow;
  for (auto item : _items) {
    data_m->progressBar->setValue(data_m->progressBar->value() +
                                  100 / _items.size());
    if (!item.first->exec()) {
      estream << "Error in " << QString("%1 %2").arg(__FILE__).arg(item.second)
              << " function\n";
      return false;
    }
  }
  data_m->customplot_1->replot();
  data_m->progressBar->setValue(100);
  QThread::msleep(200);
  data_m->progressBar->hide();
  return true;
}

}  // namespace Main

bool Profile::Item::exec() {
  return (
      Axes::Ptr::create(_data)->exec() && Earth::Ptr::create(_data)->exec() &&
      Curve::Ptr::create(_data)->exec() &&
      Fresnel::Ptr::create(_data)->exec() && Los::Ptr::create(_data)->exec());
}

bool Diagram::Item::exec() {
  return true /*(Axes::Ptr::create(_data)->exec())*/;
}

bool Atten::Land::Item::exec() {
  switch (data->interval_type) {
    case 1:
      data->mainWindow->label_intervalType->setText(QObject::tr("Открытый"));
      Opened::Ptr::create(_data)->exec();
      break;
    case 2:
      data->mainWindow->label_intervalType->setText(
          QObject::tr("Полуоткрытый"));
      SemiOpened::Ptr::create(_data)->exec();
      break;
    case 3:
      data->mainWindow->label_intervalType->setText(QObject::tr("Закрытый"));
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
  //#ifndef Q_OS_WINDOWS
  //  system(QString("sed -i '/^*$/d' %1").arg(data->filename).toStdString());
  // endif
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
  data->tower.sender.first = coords.b_x();
  data->tower.reciever.first = coords.e_x();
  data->constant.area_length = coords.e_x() - coords.b_x();
  data->param.los = strLineEquation(
      data->tower.sender.first, data->tower.sender.second + coords.b_y(),
      data->tower.reciever.first, data->tower.reciever.second + coords.e_y());
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
  constexpr int window_add_height = 45;

  QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
  cp->rescaleAxes();
  cp->setInteractions(QCP::iSelectPlottables | QCP::iRangeDrag |
                      QCP::iRangeZoom);

  for (double i = 0; i < data->constant.area_length + 500; i += 500) {
    QString str = QString::number(static_cast<int>(i) / 1000);
    textTicker->addTick(i, str);
  }

  QVector<double> heights;
  coords.y(heights);
  double h_max = *std::max_element(heights.begin(), heights.end());
  double y_max =  ///< Высота видимости графика
      std::max(h_max, std::max(coords.b_y() + data->tower.sender.second,
                               coords.e_y() + data->tower.reciever.second)) +
      window_add_height;
  cp->yAxis->setLabel("Высота, м.");
  cp->yAxis->setRange(0, y_max, Qt::AlignLeft);
  cp->xAxis->setLabel("Расстояние, км.");
  cp->xAxis->setTicker(textTicker);
  cp->xAxis->setTickLength(0);
  cp->yAxis->setSubTickLength(0);
  cp->xAxis->setRange(0, data->constant.area_length);

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
  QPen pen(QColor("#014506"), 2);

  double half =  ///< Половина длины рассматриваемого участка
      data->constant.area_length / 2;
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
  Etc::setGraph(cp, 0, x, y, pen);
  x.clear(), y.clear();
  assert(x.size() == y.size());

  if (!_data) return false;
  return true;
}

bool Curve::exec() {
  QVector<double> x, y;
  RESERVE(x, y, data->param.count);
  QPen pen(QColor("#137ea8"), 2);
  coords.x(x);
  coords.y(y);
  Etc::setGraph(cp, 1, x, y, pen);
  x.clear(), y.clear();

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
  Etc::setGraph(cp, 2, x, y, pen);

  decltype(x)::ConstIterator it = x.begin();
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
  RESERVE(x, y, data->param.count);
  QPen pen(QColor("#d6ba06"), 2);

  data->param.coords.x(x);
  for (auto it : x)
    y.push_back(data->param.los.first * it + data->param.los.second);
  Etc::setGraph(cp, 4, x, y, pen);
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
  double opposite_y_coord =  ///< Ордината точки, зеркальной к передатчику
                             ///< относительно высотного профиля
      coords.b_y() - data->tower.sender.second;
  auto pair = strLineEquation(coords.b_x(), opposite_y_coord, coords.e_x(),
                              data->tower.reciever.second + coords.e_y());
  //  QCPItemLine *line = new QCPItemLine(data->mainWindow->customplot_1);
  //  line->start->setCoords(coords.b_x(), opposite_y_coord);
  //  line->end->setCoords(coords.e_x(),
  //                       data->tower.reciever.second + coords.e_y());
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
  data->mainWindow->label_attentuationValue->setText(
      QString("%1").arg(std::round(atten * 100) / 100));
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
  auto line_l = strLineEquation(coords.b_x(), coords.b_y(), p.first, p.second);
  auto line_r = strLineEquation(coords.e_x(), coords.e_y(), p.first, p.second);

  auto poi = _pointOfIntersection(line_l, line_r);
  double h =  ///< Возвышение препятствия над ЛПВ
      poi.second - data->param.los.first * poi.first - data->param.los.second;
  double d1 =  ///< Расстояние от левого конца трассы интервала до препятствия
      qSqrt(qPow(poi.second - coords.b_y(), 2) +
            qPow(poi.first - coords.b_x(), 2));
  double d2 =  ///< Расстояние от правого конца трассы интервала до препятствия
      qSqrt(qPow(poi.second - coords.e_y(), 2) +
            qPow(poi.first - coords.e_x(), 2));
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
  data->mainWindow->label_attentuationValue->setText(
      QString("%1").arg(std::round(atten * 100) / 100));
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
  for (const auto &it : p) {
    double max_x = -1, max_y = -1;
    for (auto i = it.first; i <= it.second; ++i) {
      if (max_y < coords[i]) max_y = coords[i], max_x = i;
    }
    peaks.push_back({max_x, max_y});
  }

  LOOP_START(peaks.begin(), peaks.end(), it);
  left = (it == peaks.begin())
             ? qMakePair(data->tower.sender.first,
                         data->tower.sender.second + data->param.coords.b_y())
             : *(it - 1);
  right =
      (it == peaks.end() - 1)
          ? qMakePair(data->tower.reciever.first,
                      data->tower.reciever.second + data->param.coords.e_y())
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
    if (_isTangent(line_l.first, line_l.second, it.key(), coords.e_x())) break;
  }

  // Поиск касательной со стороны правого препятствия
  for (auto it = (coords.lowerBound(right.first) - 1); it != coords.begin();
       --it) {
    line_r = strLineEquation(right.first, right.second, it.key(), it.value());
    if (_isTangent(line_r.first, line_r.second, coords.b_x(), it.key())) break;
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
  double f = data->spec.f;
  double temperature =
      data->mainWindow->lineEdit_temperature->text().toDouble();
  double gamma_oxygen =
      (6.09 / (f * f - .227) + 7.19e-3 + 4.81 / (qPow(f - 57, 2) + 1.5)) * f *
      f * 1e-3;
  double gamma_water =
      (.05 + 3.6 / (qPow(f - 22.2, 2) + 8.5) + 0.0021 * 7.5 +
       10.6 / (qPow(f - 188.3, 2) + 9) + 8.9 / (qPow(f - 325.4, 2) + 26.3)) *
      f * f * 7.5 * 1e-4;
  data->wa = data->constant.area_length *
             ((1 - (temperature - 15) * .01) * gamma_oxygen +
              (1 - (temperature - 15) * .06) * gamma_water);
  if (!_data) return false;
  return true;
}

bool Feeder::Item::exec() {
  QString feeder_t = data->mainWindow->comboBox_feeder->currentText();
  //  double freq = data->mainWindow->comboBox_freq->currentText().toDouble();
  //  data->wf = {data->constant.linear_atten[feeder_t][freq] *
  //                  (data->tower.sender.second + data->tower.senderF_length),
  //              data->constant.linear_atten[feeder_t][freq] *
  //                  (data->tower.reciever.second +
  //                  data->tower.recieverF_length)};
  //  ostream << data->wf.first << ' ';
  if (!_data) return false;
  return true;
}

}  // namespace Atten

namespace Median {

bool Item::exec() {
  constexpr int p = 20, g1 = 2, g2 = 2;
  auto data = _data.toStrongRef();
  data->p = p - data->wf.first + g1 - (data->wp + data->ws + data->wa) + g2 -
            data->wf.second;
  if (!_data) return false;
  return true;
}

}  // namespace Median

namespace Diagram {

bool Axes::exec() {
  cp->xAxis->setVisible(0);
  cp->yAxis->setVisible(0);
  if (!_data) return false;
  return true;
}

}  // namespace Diagram

Core::Core(Ui::NRrlsMainWindow *m, const QString &filename) {
  _data = QSharedPointer<Data>::create();
  _data->mainWindow = m;
  _data->filename = filename;
  _main = Main::Item::Ptr::create(_data);
}

bool Core::exec() {
  _data->mainWindow->progressBar->show();
  _data->mainWindow->progressBar->setValue(0);
  return _main->exec();
}

void Core::setFreq(double f) {
  _data->spec.f = f;
  _data->constant.lambda = (double)3e+8 / (f * 1e+6);
}

void Core::setSenHeight(double h) { _data->tower.sender.second = h; }

void Core::setRecHeight(double h) { _data->tower.reciever.second = h; }

void Core::setSenFLength(double l) { _data->tower.senderF_length = l; }

void Core::setRecFLength(double l) { _data->tower.recieverF_length = l; }

}  // namespace Calc

}  // namespace NRrls
