#ifndef NRRLSCALC_H
#define NRRLSCALC_H

#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QSharedPointer>
#include <QVector>
#include <cassert>
#include <iostream>
#include "qcustomplot.h"

#define QSHDEF(x) typedef QSharedPointer<x> Ptr

namespace NRrls {

namespace Calc {

namespace Const {

/**
 * Константы для расчета
 */
struct Data {
  double radius = 6.37e+06;  ///< Действительный радиус Земли (в метрах)
  double g_standard = -8e-08;  ///< Вертикальный градиент индекса преломления в
                               ///< Приземной части тропосферы
  double lambda = 0.2;  ///< Длина волны
  double area_length;  ///< Длина рассматриваемого участка (в метрах)
};

}  // namespace Const

namespace Spec {

/**
 * Параметры РРЛС
 */
struct Data {
  double f = 0;  ///< Частота
  double p = 0;  ///< Пороговое значение сигнала на входе приёмника
};

}  // namespace Spec

namespace Towers {

/**
 * Параметры антенн
 */
struct Data {
  QPair<double, double> sender;  ///< Координаты передатчика
  QPair<double, double> reciever;  ///< Координаты приемника
};

}  // namespace Towers

namespace Profile {

/**
 * Параметры высотного профиля
 */
struct Data {
  typedef std::pair<double, double> Coords;
  QVector<Coords> coords;  ///< Координаты
  QPair<double, double> los;  ///< Уравнение линии прямой видимости (ЛПВ)
  QMap<double, double>
      HNull_hNull_div;  ///< Отношения критических и относительных просветов
  QMap<double, double> h_null;  ///< Относительные просветы
  QMap<double, double> H_null;  ///< Критические просветы
  QMap<double, double> H;  ///< Расстояние между ЛПВ и линией профиля местности
  size_t count;       ///< Количество точек разбиения
};

}  // namespace Profile

/**
 * Данные для расчета
 */
struct Data {
  typedef QSharedPointer<Data> Ptr;
  typedef QWeakPointer<Data> WeakPtr;

  Const::Data constant;  ///< Константы
  Spec::Data spec;       ///< Параметры РРЛ
  Profile::Data param;  ///< Параметры высотного профиля
  Towers::Data tower;   ///< Параметры антенн

  int interval_type =
      0;  ///< Тип интервала: 1-Открытый, 2-Полуоткрытый, 3-Закрытый
  double wp = 0;  ///< Затухания в рельефе
  double ws = 0;  ///< Затухания в свободном пространстве
  double wa = 0;  ///< Затухания в газах атмосферы
  double p = 0;  ///< Медианное значение сигнала на входе приёмника
};

/**
 * Составляющая расчета. Базовый класс
 */
class Item {
 public:
  QSHDEF(Item);
  Item(const Data::WeakPtr &data) : _data(data) {}
  Item(const Data::WeakPtr &data, QCustomPlot *cp) : _data(data), _cp(cp) {}

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
  double k(double R);

  /**
   * Функция вычисления протяженности участка отражения
   * @param h0 - относительный просвет в точке
   * @param k - относительная координата
   * @return Длина участка отражения
   */
  double lNull(double h0, double k);

  /**
   * Функция вычисления критического просвета
   * @param i - индекс точки, в которой считается критический просвет
   * @return Значение критического просвета в точке
   */
  double HNull(int i);

  /**
   * Функция нахождения радиуса сферы препятствия
   * @param l0 - длина участка отражения
   * @param delta_y - высота хорды между параболой(аппроксимацией препятствия) и
   * горизонталью
   * @return Радиус сферы препятствия
   */
  double obstacleSphereRadius(double l0, double delta_y);

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
  QCustomPlot *_cp;
};

namespace Master {

/**
 * Составляющая расчета, с вложениями
 */
class Item : public Calc::Item {
 public:
  QSHDEF(Item);
  Item(const Data::WeakPtr &data, QCustomPlot *cp) : Calc::Item(data, cp) {}

 protected:
  QList<QPair<Calc::Item::Ptr, QString>> _items;
};

}  // namespace Master

namespace Main {

/**
 * Составляющая расчета. Головной расчет
 */
class Item : public Master::Item {
 public:
  QSHDEF(Item);
  Item(const Data::WeakPtr &data, QCustomPlot *cp);

  // NRrls::Calc::Item interface
 public:
  virtual bool exec();
};

}  // namespace Main

/**
 * Расчет радиовидимости для РРЛС
 */
class Core {
 public:
  Core(QCustomPlot *cp);

 public:
  virtual bool exec();

 private:
  Data::Ptr _data;
  QCustomPlot *_cp;
  Main::Item::Ptr _main;
};

}  // namespace Calc

}  // namespace NRrls

#endif  // NRRLSCALC_H
