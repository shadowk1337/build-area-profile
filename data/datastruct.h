#ifndef DATASTRUCT_H
#define DATASTRUCT_H
#include "headings.h"

struct Data {
  qint32 s_counter;
  qreal s_intervals_difference;
  std::map<qreal, qreal> s_map;
  QVector<qreal> s_heights, s_HNull_hNull_div, s_hNull, s_HNull, s_H;
};

#endif  // DATASTRUCT_H
