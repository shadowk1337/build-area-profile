#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <QtGlobal>

namespace constants{
  const qreal REAL_RADIUS =
      6.37e+06;  // действительный радиус Земли (в метрах)
  const qreal G_STANDARD =
      -8e-08;  // вертикальный градиент индекса преломления
               // в приземной части тропосферы
  const qreal R_EVALUATED =
      16.9e+03;  // предельное удаление края дуги от центра (в метрах)
  const qreal AREA_LENGTH = 2 * R_EVALUATED;
  const qreal LAMBDA = 0.2;  // длина волны
}

#endif // CONSTANTS_H
