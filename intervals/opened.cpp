#include "constants.h"
#include "datastruct.h"
#include "intervals.h"
#include <calcformulas.h>

extern struct Data data_usage;

OpenedInterval::OpenedInterval() : Interval() {}

OpenedInterval::OpenedInterval(const QVector<qint32> &v) : Interval(v) {}

void OpenedInterval::IntervalType(QCustomPlot *cp, const QVector<qint32> &interval_type) {
  qint32 idx_interval_start, idx_interval_end, prev, l0_length;
  idx_interval_start = prev = interval_type[0];

  for (auto it : interval_type) {
    if (abs(it - prev) > 1 || it == *(interval_type.end() - 1)) {
      idx_interval_end = prev;
      qint32 difference = idx_interval_end - idx_interval_start;

      for (qint32 i = idx_interval_start + difference / 4;
           i <= idx_interval_end; i += difference / 4 + 1) {
        auto a = i - l0_length / (2 * data_usage.s_intervals_difference);
        auto b = i + l0_length / (2 * data_usage.s_intervals_difference);
        if ((l0_length = l0(data_usage.s_hNull[i],
                            i * data_usage.s_intervals_difference /
                                (constants::AREA_LENGTH))) <=
            0.25 * constants::AREA_LENGTH) {
          openedIntervalPlaneApproximation(
              std::max(static_cast<qint32>(a), 0),
              std::min(static_cast<qint32>(b),
                       static_cast<qint32>(constants::AREA_LENGTH)),
              i);
        } else {
          openedIntervalSphereApproximation(
              std::max(static_cast<qint32>(a), 0),
              std::min(static_cast<qint32>(b),
                       static_cast<qint32>(constants::AREA_LENGTH)),
              i);
        }
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

void OpenedInterval::openedIntervalSphereApproximation(qint32 line_start, qint32 line_end,
                                       qint32 idx_line_mid) {
  QVector<qreal> H0_h0_div = data_usage.s_HNull_hNull_div;
  auto max_H0_h0 = 0.75 * H0_h0_div.at(idx_line_mid);
  for (auto i = line_start; i < line_end; i++) {
        if (H0_h0_div.at(i) < max_H0_h0) qDebug() << "Гладкая";
        else
          qDebug() << "-";
  }
}

void OpenedInterval::rayleighAndGroundCriteria(qint32 line_start, qint32 line_end,
                               qint32 idx_line_mid) {
  qreal k, b, denominator, y, delta_h;
  auto heights = data_usage.s_heights;
  denominator = (line_end - line_start);
  k = (heights.at(line_end) -  // TODO
       heights.at(line_start) / denominator);
  b = (line_end * heights.at(line_start) - line_start * heights.at(line_end)) /
      denominator;
  auto max_H0_h0 = 0.75 * data_usage.s_HNull_hNull_div.at(idx_line_mid);
  for (auto i = line_start; i <= line_end; ++i) {
    y = k * i + b;
    delta_h = abs(y - heights.at(i));
    //    if (delta_h < 0.75 * max_H0_h0) qDebug() << i << ": Гладкая";
  }
}
