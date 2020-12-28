#ifndef DATASTRUCT_H
#define DATASTRUCT_H
#include "headings.h"

struct Data {
  qreal intervals_difference;
  QVector<qreal> indexes;
  QVector<qreal> heights, HNull_hNull_div, h_null, H_null, H;
  QVector<qreal> los_heights;  // los - line of sight
};

struct SenRecCoords {
  qreal x_sender, x_reciever, y_sender, y_reciever;
  qreal x_diff, y_diff;
};

#endif  // DATASTRUCT_H
