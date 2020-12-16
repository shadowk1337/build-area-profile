#include "constants.h"
#include "datastruct.h"
#include "intervals.h"

extern struct Data s_data;

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

qint32 ClosedInterval::findLongestInterval() const {
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

void findDeltaY(QCustomPlot *, qreal, qreal, qreal, qreal, qreal, qreal);

void ClosedInterval::reliefTangentStraightLines(QCustomPlot *cp,
                                                qint32 int_start,
                                                qint32 int_end) {
  qint32 int_end_rec, int_end_send;
  qreal dist = s_data.intervals_difference;
  bool is_higher_sender, is_higher_reciever;
  is_higher_sender = is_higher_reciever = 1;

  for (auto i = int_start; i <= int_end; ++i) {
    auto [a_sender, b_sender] =
        strLineEquation(0, 117.49, i * dist, s_data.heights.at(i));
    auto [a_reciever, b_reciever] = strLineEquation(
        constants::AREA_LENGTH, 52.7, i * dist, s_data.heights.at(i));

    if (checkTangentLine(int_start, int_end, a_sender, b_sender)) {
      int_end_send = int_end;
      QCPItemLine *line = new QCPItemLine(cp);
      line->start->setCoords(0, 117.49);
      line->end->setCoords(i * dist, s_data.heights.at(i));
    }

    if (checkTangentLine(int_start, int_end, a_reciever, b_reciever)) {
      int_end_rec = int_end;
      QCPItemLine *line = new QCPItemLine(cp);
      line->start->setCoords(constants::AREA_LENGTH, 52.7);
      line->end->setCoords(i * dist, s_data.heights.at(i));
    }
    findDeltaY(cp, a_sender, b_sender, int_end_send, a_reciever, b_reciever,
               int_end_rec);
  }
}

// y = ax + b
std::pair<qreal, qreal> ClosedInterval::strLineEquation(qreal x, qreal y,
                                                        qreal x_relief,
                                                        qreal y_relief) const {
  qreal c = (y_relief - y);
  qreal d = (x_relief - x);
  return {c / d, y - (x * c / d)};
}

bool ClosedInterval::checkTangentLine(qint32 int_start, qint32 int_end, qreal a,
                                      qreal b) const {
  for (auto j = int_start; j <= int_end; ++j) {
    if (a * j * s_data.intervals_difference + b < s_data.heights.at(j))
      return false;
  }
  return true;
}

void findDeltaY(QCustomPlot *cp, qreal a_sender, qreal b_sender,
                qreal end_sender, qreal a_reciever, qreal b_reciever,
                qreal end_reciever) {
  static qint32 j = 3;
  auto min_height =
      std::max_element(s_data.heights.begin(), s_data.heights.end());
  qDebug() << a_sender << b_sender;
  QVector<qreal> x, y;
  for (qint32 i = 0; i <= end_sender; ++i) {
    x.push_back(i * s_data.intervals_difference);
    y.push_back(a_sender * i * s_data.intervals_difference + b_sender -
                qAbs(s_data.H_null.at(i)));
    if (y.at(i) < s_data.heights.at(i) + 1 &&
        y.at(i) > s_data.heights.at(i) - 1)
      *min_height = std::min(*min_height, s_data.heights.at(i));
  }
  for (qint32 i = s_data.H_null.size() - 1; i >= end_reciever; --i){ // TODO: доделать

    }
  qDebug() << *min_height;

  cp->addGraph();
  cp->graph(j++)->setData(x, y);
  cp->replot();
}
