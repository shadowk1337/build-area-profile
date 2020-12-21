#ifndef DATASTRUCT_H
#define DATASTRUCT_H
#include "headings.h"

struct Data {
  qint32 counter;
  qreal intervals_difference;
  std::map<qreal, qreal> map;
  QVector<qreal> heights, HNull_hNull_div, h_null, H_null, H;
};

struct SenRecCoords {
  qreal x_sender, x_reciever, y_sender, y_reciever;
  qreal x_diff, y_diff;
};

#endif  // DATASTRUCT_H
