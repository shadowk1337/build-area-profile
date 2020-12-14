#ifndef INTERVALS_H
#define INTERVALS_H
#include "headings.h"

// TODO: сделать классы
namespace OpenedI {}

namespace HalfopenedI {}

//namespace ClosedI {
  void closedInterval(QCustomPlot *, const QVector<qint32> &);
  qint32 findLongestInterval(const QVector<qint32> &v);
  void reliefTangentStraightLines(QCustomPlot *, qint32, qint32);
  std::pair<qreal, qreal> strLineEquation(qreal x, qreal y, qreal x_relief,
                                          qreal y_relief);


//}

#endif  // INTERVALS_H
