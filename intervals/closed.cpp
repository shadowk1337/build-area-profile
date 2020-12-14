#include "datastruct.h"
#include "constants.h"
#include "intervals.h"



void closedInterval(QCustomPlot *cp, const QVector<qint32> &interval_type) {
  qint32 idx_interval_start, prev = 0;
  idx_interval_start = prev = interval_type[0];
  auto max = findLongestInterval(interval_type) / 2;

  for (auto it : interval_type) {
    if (it - prev >= max && prev == *interval_type.begin())
      idx_interval_start = it;
    if ((it - prev >= max && prev != *interval_type.begin()) ||
        it == *(interval_type.end() - 1)) {
      reliefTangentStraightLines(cp, idx_interval_start, prev);
      idx_interval_start = it;
    }
    prev = it;
  }
}

qint32 findLongestInterval(const QVector<qint32> &v) {
  qint32 a, int_begin = v.at(0), prev = v.at(0), longestSize = 0;
  for (qint32 i = 0; i < v.size(); ++i) {
    if (v.at(i) - prev > 1) {
      a = prev - int_begin;
      longestSize = (a > longestSize) ? a : longestSize;
      int_begin = v.at(i);
    }
    prev = v.at(i);
  }
  return longestSize;
}

inline std::pair<qreal, qreal> strLineEquation(qreal, qreal, qreal, qreal);
bool checkTangentLine(qint32, qint32, qreal, qreal);

void reliefTangentStraightLines(QCustomPlot *cp, qint32 int_start,
                                qint32 int_end) {
  qreal dist = data_usage.s_intervals_difference;
  bool is_higher_sender, is_higher_reciever;
  is_higher_sender = is_higher_reciever = 1;
  for (auto it : data_usage.s_heights)
    qDebug() << it;

  for (auto i = int_start; i <= int_end; ++i) {
    auto [a_sender, b_sender] =
        strLineEquation(0, 117.49, i * dist, data_usage.s_heights.at(i));
    auto [a_reciever, b_reciever] = strLineEquation(
        constants::AREA_LENGTH, 52.7, i * dist, data_usage.s_heights.at(i));

    if (checkTangentLine(int_start, int_end, a_sender, b_sender)) {
      QCPItemLine *line = new QCPItemLine(cp);
      line->start->setCoords(0, 117.49);
      line->end->setCoords(i * dist, data_usage.s_heights.at(i));
    }

    if (checkTangentLine(int_start, int_end, a_reciever, b_reciever)) {
      QCPItemLine *line = new QCPItemLine(cp);
      line->start->setCoords(constants::AREA_LENGTH, 52.7);
      line->end->setCoords(i * dist, data_usage.s_heights.at(i));
    }
  }
}

// y = ax + b
std::pair<qreal, qreal> strLineEquation(qreal x, qreal y, qreal x_relief,
                                        qreal y_relief) {
  qreal c = (y_relief - y);
  qreal d = (x_relief - x);
  return {c / d, y - (x * c / d)};
}

bool checkTangentLine(qint32 int_start, qint32 int_end, qreal a, qreal b) {
  for (auto j = int_start; j <= int_end; ++j) {
    if (a * j * data_usage.s_intervals_difference + b <
        data_usage.s_heights.at(j))
      return false;
  }
  return true;
}
