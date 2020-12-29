#include "intervals.h"

Interval::Interval() {}

// Составление уравнения прямой
std::pair<qreal, qreal> Interval::strLineEquation(qreal x, qreal y,
                                                  qreal x_relief,
                                                  qreal y_relief) const {
  qreal c = (y_relief - y);
  qreal d = (x_relief - x);
  return {c / d, y - (x * c / d)};
}
