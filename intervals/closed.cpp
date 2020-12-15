#include "constants.h"
#include "datastruct.h"
#include "intervals.h"

extern struct Data data_usage;

ClosedInterval::ClosedInterval() : Interval() {}

ClosedInterval::ClosedInterval(const QVector<qint32> &v) : Interval(v) {}

void ClosedInterval::IntervalType(QCustomPlot *cp, const QVector<qint32> &v) {
  m_interval_type = v;
  qint32 idx_interval_start, prev = 0;
  idx_interval_start = prev = m_interval_type[0];
  auto max = findLongestInterval() / 2;

  for (auto it : m_interval_type) {
    if (it - prev >= max && prev == *m_interval_type.begin())
      idx_interval_start = it;
    if ((it - prev >= max && prev != *m_interval_type.begin()) ||
        it == *(m_interval_type.end() - 1)) {
      reliefTangentStraightLines(cp, idx_interval_start, prev);
      idx_interval_start = it;
    }
    prev = it;
  }
}

qint32 ClosedInterval::findLongestInterval() {
  qint32 a, int_begin = m_interval_type.at(0), prev = m_interval_type.at(0),
            longestSize = 0;
  for (qint32 i = 0; i < m_interval_type.size(); ++i) {
    if (m_interval_type.at(i) - prev > 1) {
      a = prev - int_begin;
      longestSize = (a > longestSize) ? a : longestSize;
      int_begin = m_interval_type.at(i);
    }
    prev = m_interval_type.at(i);
  }
  return longestSize;
}

void ClosedInterval::reliefTangentStraightLines(QCustomPlot *cp,
                                                qint32 int_start,
                                                qint32 int_end) {
  qreal dist = data_usage.s_intervals_difference;
  bool is_higher_sender, is_higher_reciever;
  is_higher_sender = is_higher_reciever = 1;

  //  for (auto it : data_usage.s_heights) qDebug() << it;


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
std::pair<qreal, qreal> ClosedInterval::strLineEquation(qreal x, qreal y,
                                                        qreal x_relief,
                                                        qreal y_relief) {
  qreal c = (y_relief - y);
  qreal d = (x_relief - x);
  return {c / d, y - (x * c / d)};
}

bool ClosedInterval::checkTangentLine(qint32 int_start, qint32 int_end, qreal a,
                                      qreal b) {
  for (auto j = int_start; j <= int_end; ++j) {
    if (a * j * data_usage.s_intervals_difference + b <
        data_usage.s_heights.at(j))
      return false;
  }
  return true;
}
