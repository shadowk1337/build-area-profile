#ifndef NRRLSCALC_H
#define NRRLSCALC_H

#include <cassert>
#include <cmath>
#include <iostream>
#include <utility>

#include "nrrlsgraphpainter.h"

#include "ui_nrrlscoordswindow.h"
#include "ui_nrrlsdiagramwindow.h"
#include "ui_nrrlsfirststationwidget.h"
#include "ui_nrrlsmainwindow.h"
#include "ui_nrrlssecondstationwindow.h"

#define QSHDEF(x) typedef QSharedPointer<x> Ptr

#define LOOP_START(begin, end, it) \
  loop(begin, end, [&](decltype(begin) it) {
#define LOOP_END \
  ;              \
  })

#define RESERVE(x, y, size) \
  x.reserve(size);          \
  y.reserve(size)

#define FIND (c, begin, end, f) find_if_el(c, begin, end, f)

#define C(x) (sgn(x) * log10(1 + qAbs(x) / 10))

#define C_REVERSE(x) (sgn(x) * 10 * (qPow(10, x / sgn(x)) - 1))

template <typename T>
int sgn(T val) {
  return (T(0) < val) - (val < T(0));
}

template <typename T = double>
double fromVtToDbvt(T to_dbvt) {
  return 10 * log10(to_dbvt);
}

template <typename Iterator, typename Func>
void loop(Iterator begin, Iterator end, Func f) {
  Iterator _it;
  for (_it = begin; _it != end; _it = std::next(_it)) {
    f(_it);
  }
}

namespace NRrls {
namespace Calc {

namespace Const {

/**
 * Константы для расчета
 */
struct Data {
  double g_standard = 0;  ///< Вертикальный градиент индекса
                          ///< преломления в приземной части тропосферы
  double lambda = 0;       ///< Длина волны
  double area_length = 0;  ///< Длина рассматриваемого участка (в метрах)
  const double radius = 6.37e+06;  ///< Действительный радиус Земли (в метрах)
  double temperature = 0;

  const QMap<double, QVector<double>> reflection_coef = {{{0.015, {.2, .1, .6}},
                                                          {0.03, {.45, .1, .6}},
                                                          {0.05, {.7, .2, .8}},
                                                          {0.08, {.8, .4, .85}},
                                                          {0.2, {.9, .5, .9}},
                                                          {1, {.95, .7, .95}}}};
};

}  // namespace Const

namespace Spec {

/**
 * Параметры РРЛС
 */
struct Data {
  double f = 0;     ///< Частота
  double q = 0;     ///< Надежность (Запас) связи
  double prob = 0;  ///< Вероятность связи
  double sesrg = 0;
  double sesrc = 0;
  QPair<double, double> p = {0, 0};  ///< Мощность
  QPair<double, double> s = {0, 0};  ///< Чувствительность

  using CapacityNCoef = QVector<double>;

  const QMap<QString, CapacityNCoef> stat = {
      {QObject::tr("Р-419МЦ"), {48, 10}}};

  using Sensitivity = QMap<QString, double>;

  QMap<QString, Sensitivity> j = {
      {QObject::tr("Р-419МЦ"),
       {{QObject::tr("68/136"), 4},
        {QObject::tr("85/170"), 7},
        {QObject::tr("68/272"), 4},
        {QObject::tr("544/544"), 12},
        {QObject::tr("2176"), 17},
        {QObject::tr("При работе с модемом Е2"), 19},
        {QObject::tr("Ц48"), 9},
        {QObject::tr("Ц480"), 19},
        {QObject::tr("А6-4"), 7},
        {QObject::tr("БУК"), 6},
        {QObject::tr("А6-5"), 10}}}};

  using GraphDots = QVector<QPointF>;

  const QMap<double, GraphDots> depend = {
      {100, {{11, 0}, {10, .04}, {8, .3}, {6, 2}, {4, 10}, {0, 50}}},
      {200, {{15, 0}, {14, .035}, {12, .2}, {8, 2}, {6, 6}, {4, 16}, {0, 50}}},
      {400, {{18.2, 0}, {16, .055}, {12, .55}, {8, 3.9}, {6, 11}, {0, 50}}},
      {800,
       {{25, 0},
        {20, .1},
        {18, .2},
        {16, .4},
        {12, 1.5},
        {10, 3},
        {8, 7},
        {4, 24},
        {0, 50}}},
      {2000,
       {{31.2, 0},
        {26, .004},
        {20, .19},
        {18, .49},
        {16, .79},
        {10, 4.9},
        {4, 24},
        {0, 50}}},
      {4000,
       {{35.2, 0},
        {26, .11},
        {24, .18},
        {18, .7},
        {12, 4.8},
        {10, 6},
        {6, 16.2},
        {0, 50}}},
      {6000,
       {{36, .022},
        {32, .05},
        {24, .32},
        {20, .7},
        {16, 2},
        {12, 5},
        {10, 8},
        {6, 20},
        {0, 50}}},
      {8000,
       {{36, .054},
        {30, .18},
        {24, .5},
        {20, 1},
        {12, 6},
        {10, 10},
        {6, 20},
        {4, 30},
        {0, 50}}}};
};

}  // namespace Spec

namespace Towers {

/**
 * Параметры антенн
 */
struct Data {
  QPointF f = {0, 0};  ///< Координаты первой антенны
  QPointF s = {0, 0};  ///< Координаты второй антенны
  QPair<double, double> c = {0, 0};  ///< Коэффициент усиления
  QPair<double, double> wf = {0,
                              0};  ///< Затухания в фидерах на передачу и прием
};

}  // namespace Towers

namespace Profile {

/**
 * Параметры высотного профиля
 */
struct Data {
  class {
   public:
    double startX(void) { return _coords.firstKey(); }
    double startY(void) { return _coords.first(); }
    double endX(void) { return _coords.lastKey(); }
    double endY(void) { return _coords.last(); }

    void x(QVector<double> &v) const {
      v.reserve(_coords.size());
      foreach (auto i, _coords.keys())
        v.push_back(i);
    }

    void y(QVector<double> &v) const {
      v.reserve(_coords.size());
      foreach (auto i, _coords.values())
        v.push_back(i);
    }

    const QList<double> &x(void) { return std::move(_coords.keys()); }

    const QList<double> &y(void) { return std::move(_coords.values()); }

    QMap<double, double>::iterator begin() { return _coords.begin(); }

    QMap<double, double>::iterator end() { return _coords.end(); }

    std::map<double, double> toMap(void) const { return _coords.toStdMap(); }

    QMap<double, double> M(void) const { return _coords; }

    QMap<double, double>::const_iterator lowerBound(double key) const {
      return _coords.lowerBound(key);
    }

    double &operator[](double x) { return _coords[x]; }

   private:
    QMap<double, double> _coords;  ///< Координаты высотного профиля+
  } coords;
  QMap<double, double> coordsAndEarth;  ///< Координаты высотного профиля с
                                        ///< учетом земной поверхности
  QPair<double, double> los;  ///< Уравнение линии прямой видимости (ЛПВ)
  QMap<double, double> h_null;  ///< Относительные просветы
  QMap<double, double> H_null;  ///< Критические просветы
  QMap<double, double> H;  ///< Расстояние между ЛПВ и линией профиля местности
  size_t count;  ///< Количество точек разбиения
};

}  // namespace Profile

/**
 * Данные для расчета
 */
struct Data {
  typedef QSharedPointer<Data> Ptr;
  typedef QWeakPointer<Data> WeakPtr;

  Const::Data constant;  ///< Константы
  Profile::Data param;  ///< Параметры высотного профиля
  Spec::Data spec;      ///< Параметры РРЛС
  Towers::Data tower;   ///< Параметры антенн

  Ui::NRrlsMainWindow *mainWindow;

  QSharedPointer<GraphPainter> gr;

  int fr_up_idx, fr_dw_idx;  ///< Индексы графиков зоны Френеля

  QString filename;
  int interval_type;  ///< Тип интервала: 1-Открытый, 2-Полуоткрытый,
                      ///< 3-Закрытый
  double wp = 0;  ///< Затухания в рельефе
  double ws = 0;  ///< Затухания в свободном пространстве
  double wa = 0;  ///< Затухания в газах атмосферы
  QPair<double, double> p = {0, 0};  ///< Медианное значение сигнала на входе
                                     ///< приёмника
  QPair<double, double> log_p = {0,
                                 0};  ///< Медианное значение сигнала на входе
                                      ///< приёмника в логарифмическом виде
};

/**
 * Составляющая расчета. Базовый класс
 */
class Item {
 public:
  QSHDEF(Item);
  Item(const Data::WeakPtr &data) : _data(data) {}

  virtual ~Item() {}

 public:
  /**
   * Запуск расчета
   * @return Признак завершения расчета
   */
  virtual bool exec() = 0;

 protected:
  /**
   * Функция вычисления относительной координаты
   * @param R - расстояние от начала интервала до рассматриваемой точки
   * @return Величина относительной координаты
   */
  double k(double R) const;

  /**
   * Функция вычисления протяженности участка отражения
   * @param h0 - относительный просвет в точке
   * @param k - относительная координата
   * @return Длина участка отражения
   */
  double lNull(double h0, double k) const;

  /**
   * Функция вычисления критического просвета
   * @param i - индекс точки, в которой считается критический просвет
   * @return Значение критического просвета в точке
   */
  double HNull(double i) const;

  /**
   * Функция нахождения радиуса сферы препятствия
   * @param l0 - длина участка отражения
   * @param delta_y - высота хорды между параболой(аппроксимацией
   * препятствия) и горизонталью
   * @return Радиус сферы препятствия
   */
  double obstacleSphereRadius(double l0, double delta_y) const;

  /**
   * Функция построения уравнения прямой y = $a * x + $b
   * @param x       - абсцисса первой точки
   * @param y       - ордината первой точки
   * @param xx      - абсцисса второй точки
   * @param yy      - ордината второй точки
   * @return Пара{$a, $b}
   */
  QPair<double, double> strLineEquation(double x, double y, double xx,
                                        double yy) const;

 protected:
  Data::WeakPtr _data;
};

namespace Master {

/**
 * Составляющая расчета, с вложениями
 */
class Item : public Calc::Item {
 public:
  QSHDEF(Item);
  Item(const Data::WeakPtr &data) : Calc::Item(data) {}

 protected:
  QList<Calc::Item::Ptr> _items;
};

}  // namespace Master

namespace Main {

/**
 * Составляющая расчета. Головной расчет
 */
class Item : public Master::Item {
 public:
  QSHDEF(Item);
  Item(const Data::WeakPtr &data);

 public:
  virtual bool exec();
};

}  // namespace Main

class Core {
 public:
  Core(Ui::NRrlsMainWindow *m, const QString &filename);

 public:
  virtual bool exec();
  void setFreq(double f);

  template <typename T = double>
  void setValue(double &to, double v) {
    to = v;
  };

  template <typename T = double>
  T setValueWithReturn(T &to, T v) {
    to = v;
    return v;
  };

  double coordX(double c);
  double coordY(double c);

  double xRange();
  double yRange();

 public:
  Data::Ptr data;

 private:
  QCustomPlot *_cp;
  Main::Item::Ptr _main;
};

}  // namespace Calc

}  // namespace NRrls

#endif  // NRRLSCALC_H
