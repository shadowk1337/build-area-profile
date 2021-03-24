#ifndef NRRLSCALC_H
#define NRRLSCALC_H

#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QSharedPointer>
#include <QVector>
#include <cassert>
#include <cmath>
#include <iostream>
#include <utility>
#include "qcustomplot.h"

#include "ui_nrrlsmainwindow.h"

#define C(x) sgn(x) * log10(1 + qAbs(x) / 10)

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

template <typename T>
int sgn(T val) {
  return (T(0) < val) - (val < T(0));
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
  double g_standard = -8e-08;  ///< Вертикальный градиент индекса
                               ///< преломления в Приземной части тропосферы
  double lambda = 0;       ///< Длина волны
  double area_length = 0;  ///< Длина рассматриваемого участка (в метрах)
  const double radius = 6.37e+06;  ///< Действительный радиус Земли (в метрах)
  double temperature = 0;

  const QMap<QString, QMap<double, double>>
      linear_atten =  ///< Погонные затухания в фидерах
      {{"РК-50-4-13", {{.1, .1}, {1, .5}, {3, .9}, {6, 1.4}, {10, 2}}},
       {"РК-50-7-11", {{.1, .09}, {1, .4}, {3, .8}, {6, 1.1}, {10, 1.5}}},
       {"РК-50-7-13", {{.1, .07}, {1, 0.3}, {3, .56}, {6, .8}, {10, 1.2}}},
       {"РК-50-7-25", {{.1, .06}, {1, 0.26}, {3, .46}, {6, .7}, {10, 1}}},
       {"РК-50-7-27", {{.1, .06}, {1, 0.25}, {3, .45}, {6, .65}, {10, .9}}},
       {"РК-50-7-32", {{.1, .06}, {1, 0.26}, {3, .45}, {6, .9}, {10, 1}}},
       {"РК-50-11-11", {{.1, .06}, {1, 0.29}, {3, .55}}},
       {"РК-50-13-15", {{.1, .038}, {1, 0.16}, {3, .28}}},
       {"РК-50-11-21", {{.1, .056}, {1, 0.22}, {3, .4}}},
       {"МЭК-22", {{3, .01}}},
       {"МЭК-58", {{6, .044}}},
       {"МЭК-84", {{10, .08}}},
       {"МЭК-120", {{15, .13}}},
       {"МЭК-180", {{20, .24}}},
       {"МЭК-26", {{30, .44}}}};
};

}  // namespace Const

namespace Spec {

/**
 * Параметры РРЛС
 */
struct Data {
  double f = 0;                      ///< Частота
  QPair<double, double> p = {0, 0};  ///< Мощность
  QPair<double, double> s = {0, 0};  ///< Чувствительность
  double Flength =  ///< Длина горизонтального участка фидера передатчика
      0;
  double Slength =  ///< Длина горизонтального участка фидера приемника
      0;
};

}  // namespace Spec

namespace Towers {

/**
 * Параметры антенн
 */
struct Data {
  QPair<double, double> f = {0, 0};  ///< Координаты первой антенны
  QPair<double, double> s = {0, 0};  ///< Координаты второй антенны
  QPair<double, double> c = {0, 0};  ///< Коэффициент усиления
};

}  // namespace Towers

namespace Profile {

/**
 * Параметры высотного профиля
 */
struct Data {
  class {
   public:
    double b_x(void) { return _coords.begin().key(); }
    double b_y(void) { return _coords.begin().value(); }
    double e_x(void) { return std::prev(_coords.end()).key(); }
    double e_y(void) { return std::prev(_coords.end()).value(); }

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

    const QList<double> &x(void) { return _coords.keys(); }

    const QList<double> &y(void) { return _coords.values(); }

    QMap<double, double>::const_iterator begin() const {
      return _coords.begin();
    }

    QMap<double, double>::const_iterator end() const { return _coords.end(); }

    std::map<double, double> toMap(void) const { return _coords.toStdMap(); }

    QMap<double, double>::const_iterator lowerBound(double key) const {
      return _coords.lowerBound(key);
    }

    double &last() { return _coords.last(); }

    double &operator[](double x) { return _coords[x]; }

   private:
    QMap<double, double> _coords;  ///< Координаты
  } coords;
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
  QString filename;
  int interval_type;  ///< Тип интервала: 1-Открытый, 2-Полуоткрытый,
                      ///< 3-Закрытый
  double wp = 0;  ///< Затухания в рельефе
  double ws = 0;  ///< Затухания в свободном пространстве
  double wa = 0;  ///< Затухания в газах атмосферы
  QPair<double, double> wf =  ///< Затухания в фидере на передачу и прием
      {0, 0};
  double p = 0;  ///< Медианное значение сигнала на входе приёмника в логарифмическом виде
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
  QList<QPair<Calc::Item::Ptr, int>> _items;
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
  void setFHeight(double h);
  void setSHeight(double h);
  void setFCoef(double c);
  void setSCoef(double c);
  void setFLength(double l);
  void setSLength(double l);
  void setFPower(double p);
  void setSPower(double p);
  void setFSensetivity(double s);
  void setSSensetivity(double s);
  void setTemperature(double t);

 private:
  Data::Ptr _data;
  QCustomPlot *_cp;
  Main::Item::Ptr _main;
};

}  // namespace Calc

}  // namespace NRrls

#endif  // NRRLSCALC_H
