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

/*
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
  bool exec() override { return false; }
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
      /*Interval::Item::Ptr::create(_data),
      Atten::Land::Item::Ptr::create(_data),
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

bool Fill::Item::exec() {
  QFile file("heights.csv");
  QTextStream in(&file);
  QRegExp rx("[ ;]");
  bool first = true;
  auto &heights = _data.toStrongRef()->constr.heights;
  auto &diff = _data.toStrongRef()->constr.diff;
  size_t &count = _data.toStrongRef()->constr.count;
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
  diff = (_data.toStrongRef()->cons.r_evaluated * 2) / count;
  if (_data.isNull()) return false;
  return true;
}  // namespace Calc

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
  for (qreal i = 0; i < data->cons.r_evaluated * 2 + 500; i += 500) {
    str = QString::number(static_cast<qint32>(i) / 1000);
    textTicker->addTick(i, str);
  }
  qreal h_max = *std::max_element(data->constr.heights.begin(),
                                  data->constr.heights.end()) +
                35;
  _cp->yAxis->setRange(0, h_max, Qt::AlignLeft);
  _cp->xAxis->setTicker(textTicker);
  _cp->xAxis->setTickLabelFont(pfont);
  _cp->xAxis->setTickLabelRotation(-45);
  _cp->xAxis->setTickLength(0);
  _cp->yAxis->setSubTickLength(0);
  _cp->xAxis->setRange(0, data->cons.r_evaluated * 2);
  if (_data.isNull()) return false;
  return true;
}

qreal equivalentRadius(qreal g) { return 6.37e+06 / (1 + g * 6.37e+06 / 2); }

qreal deltaY(qreal r, qreal eq_radius) { return (r * r) / (2 * eq_radius); }

bool Profile::Earth::exec() {
  auto data = _data.toStrongRef();
  QVector<qreal> x, y;
  bool first = true;
  qreal move_graph_up_value;
  qreal equivalent_radius = equivalentRadius(data->cons.g_standard);
  QPen pen(QColor("#014506"));
  pen.setWidth(2);
  for (qreal i = -data->cons.r_evaluated, iterator = 0;
       i < data->cons.r_evaluated;
       i += data->constr.diff, iterator += data->constr.diff) {
    x.push_back(i + data->cons.r_evaluated);
    if (first) {
      move_graph_up_value = qAbs(deltaY(i, equivalent_radius));
      y.push_back(0);
      first = false;
    } else
      y.push_back(-deltaY(i, equivalent_radius) + move_graph_up_value);
  }
  _cp->addGraph();
  _cp->graph(0)->setData(x, y, true);
  _cp->graph(0)->setPen(pen);
  QCPItemTracer *tracer = new QCPItemTracer(_cp);
  tracer->setGraph(_cp->graph(0));
  tracer->updatePosition();
  assert(x.size() == y.size());
  if (!_data) return false;
  return true;
}

bool Profile::Curve::exec() {
  QVector<qreal> x, y;
  qreal i = 0;
  QPen pen(QColor("#137ea8"));
  pen.setWidth(2);
  for (auto h : _data.toStrongRef()->constr.heights) {
    x.push_back(i);
    y.push_back(h);
    i += _data.toStrongRef()->constr.diff;
  }
  assert(x.size() == y.size());
  _cp->addGraph();
  _cp->graph(1)->setData(x, y);
  _cp->graph(1)->setPen(pen);
  //    lineOfSight(plot);
  //    s_data->heights = heights;
  if (!_data) return false;
  return true;
}

bool Profile::Fresnel::exec() {
  const auto &data_c = _data.toStrongRef()->constr;
  QVector<double> x, y;
  QPen pen(Qt::red);
  pen.setWidth(2);
  for (size_t i = 0; i + 1 <= data_c.count; ++i) {
    x.push_back(i * data_c.diff);
    y.push_back(-data_c.H_null.at(i) + data_c.los_heights.at(i));
  }
  y[data_c.count - 1] = data_c.reciever_coords.second;
  _cp->addGraph();
  _cp->graph(2)->setPen(pen);
  _cp->graph(2)->setData(x, y);
  std::for_each(y.begin(), y.end() - 1, [&](qreal &x) {
    static qint32 i = 0;
    x += 2 * data_c.H_null.at(i);
    i++;
  });
  y[data_c.count - 1] = data_c.reciever_coords.second;
  _cp->addGraph();
  _cp->graph(3)->setPen(pen);
  _cp->graph(3)->setData(x, y);
  if (!_data) return false;
  return true;
}

bool Profile::Los::exec() {
  auto data = _data.toStrongRef();
  QVector<qreal> y;
  auto x1 = data->constr.sender_coords.first = 0;
  auto y1 = data->constr.sender_coords.second = 117.49;
  auto x2 = data->constr.reciever_coords.first = 33700;
  auto y2 = data->constr.reciever_coords.second = 52.7;
  auto y_diff = (y2 - y1) / data->constr.heights.size();
  QPen pen(QColor("#d6ba06"));
  pen.setWidth(2);
  for (size_t i = 0; i + 1 <= data->constr.count; ++i)
    data->constr.los_heights.push_back(y1 + i * y_diff);
  QCPItemLine *line = new QCPItemLine(_cp);
  line->setPen(pen);
  line->start->setCoords(x1, y1);
  line->end->setCoords(x2, y2);
  if (!_data) return false;
  return true;
}

bool Interval::Item::exec() { return true; }

Core::Core(QCustomPlot *cp) {
  _data = QSharedPointer<Data>::create();
  _cp = cp;
  _main = Main::Item::Ptr::create(_data, _cp);
}

bool Core::exec() { return _main->exec(); }

}  // namespace Calc

}  // namespace NRrls
