#ifndef DATASTRUCT_H
#define DATASTRUCT_H
#include "headings.h"

struct Data {
  qint32 counter;
  qreal intervals_difference;
  std::map<qreal, qreal> map;
  QVector<qreal> heights, HNull_hNull_div, h_null, H_null, H;
};

#endif  // DATASTRUCT_H
