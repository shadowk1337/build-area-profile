#include "calcformules.h"
#include "constants.h"
#include "datastruct.h"
#include "intervals.h"

extern struct Data *s_data;
extern struct SenRecCoords *s_tower_coords;

OpenedInterval::OpenedInterval() : Interval() {}

OpenedInterval::OpenedInterval(const QVector<qint32> &v) : Interval(v) {}

qreal reverseSenderLineOfSight(qint32, qint32);

void OpenedInterval::IntervalType(QCustomPlot *cp,
                                  const QVector<qint32> &interval_type) {
  qint32 idx_interval_start, idx_interval_end, prev, lNull_length;
  idx_interval_start = prev = interval_type[0];

  for (auto it : interval_type) {
    if (abs(it - prev) > 1 || it == *(interval_type.end() - 1)) {
      idx_interval_end = prev;

      auto res = reverseSenderLineOfSight(idx_interval_start, idx_interval_end);
      if ((res) <= 0.25 * constants::AREA_LENGTH) {
        openedIntervalPlaneApproximation();
      } else {
        openedIntervalSphereApproximation();
      }
      idx_interval_start = it;
    }
    prev = it;
  }
}

// Открытые интервалы
void OpenedInterval::openedIntervalPlaneApproximation(qint32 idx_line_start,
                                                      qint32 idx_line_end,
                                                      qint32 idx_line_mid) {
  QVector<qreal> x, y;
  rayleighAndGroundCriteria(idx_line_start, idx_line_end, idx_line_mid);
}

void OpenedInterval::openedIntervalSphereApproximation(qint32 line_start,
                                                       qint32 line_end,
                                                       qint32 idx_line_mid) {
  QVector<qreal> H0_h0_div = s_data->HNull_hNull_div;
  auto max_H0_h0 = 0.75 * H0_h0_div.at(idx_line_mid);
  //  for (auto i = line_start; i < line_end; i++) {
  //        if (H0_h0_div.at(i) < max_H0_h0) qDebug() << "Гладкая";
  //        else
  //          qDebug() << "-";
  //  }
}

void OpenedInterval::rayleighAndGroundCriteria(qint32 line_start,
                                               qint32 line_end,
                                               qint32 idx_line_mid) {
  qreal k, b, denominator, y, delta_h;
  auto heights = s_data->heights;
  denominator = (line_end - line_start);
  k = (heights.at(line_end) -
       heights.at(line_start) / denominator);
  b = (line_end * heights.at(line_start) - line_start * heights.at(line_end)) /
      denominator;
  auto max_H0_h0 = 0.75 * s_data->HNull_hNull_div.at(idx_line_mid);
  for (auto i = line_start; i <= line_end; ++i) {
    y = k * i + b;
    delta_h = abs(y - heights.at(i));
    //    if (delta_h < 0.75 * max_H0_h0) qDebug() << i << ": Гладкая";
  }
}

qreal reverseSenderLineOfSight(qint32 int_start, qint32 int_end) {
  auto pivot = int_end / 2 + int_start / 2;
  qreal max_first = int_start, max_last = int_end;
  for (auto i = pivot; i < int_end; i++)
    if (s_data->heights.at(i - 1) <= s_data->heights.at(i) &&
        s_data->heights.at(i + 1) <= s_data->heights.at(i)) {
      max_last = i;
      break;
    }
  for (auto i = pivot; i > int_start; --i) {
    if (s_data->heights.at(i - 1) <= s_data->heights.at(i) &&
        s_data->heights.at(i + 1) <= s_data->heights.at(i)) {
      max_first = i;
      break;
    }
  }
  if (max_first && max_last) {
    return lNull(s_data->h_null.at(max_last),
                 k(max_last * s_data->intervals_difference));
  }
  return 0;
}
