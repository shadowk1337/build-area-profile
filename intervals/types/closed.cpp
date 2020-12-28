#include <algorithm>
#include "calcformules.h"
#include "constants.h"
#include "datastruct.h"
#include "intervals.h"

using namespace Closed;

extern struct Data *s_data;
extern struct SenRecCoords *s_tower_coords;

// ClosedInterval::ClosedInterval() {
//  reliefTangentStraightLines(*s_data->indexes.begin(),
//  *s_data->indexes.end());
//}

ClosedInterval::ClosedInterval(QCustomPlot *cp) {
  reliefTangentStraightLines(cp, *s_data->indexes.begin(),
                             *s_data->indexes.end());
}

qreal findlNull(qint32, qint32, std::pair<qint32, qreal>);

qint32 ClosedInterval::countPeaks(void) {
  qint32 i = 0, count = 0;
  bool inside = 0;
  for (auto it : s_data->los_heights) {
    if (it >= s_data->heights.at(i) && it <= s_data->heights.at(i + 1) &&
        inside == 0) {
      inside = 1;
    }
    if (it >= s_data->heights.at(i + 1) && it <= s_data->heights.at(i) &&
        inside == 1) {
      inside = 0;
      count++;
    }
    i++;
  }
  return count;
}

void ClosedInterval::reliefTangentStraightLines(QCustomPlot *cp,
                                                qint32 int_start,
                                                qint32 int_end) {
  qreal dist = s_data->intervals_difference;
  std::pair<qint32, qreal> min_height_send, min_height_rec;
  qreal ind_send, ind_rec;  // индексы точек касания

  for (auto i = int_start; i <= int_end; ++i) {
    auto [a_sender, b_sender] =
        strLineEquation(0, 117.49, i * dist, s_data->heights.at(i));
    auto [a_reciever, b_reciever] = strLineEquation(
        constants::AREA_LENGTH, 52.7, i * dist, s_data->heights.at(i));
    if (isTangent(int_start, int_end, a_sender, b_sender)) {

      ind_send = i;
      min_height_send = findMinHeight(a_sender, b_sender, int_start, int_end);
    }
    if (isTangent(int_start, int_end, a_reciever, b_reciever)) {
      ind_rec = i;
      min_height_rec =
          findMinHeight(a_reciever, b_reciever, int_start, int_end);
    }
  }
//  qDebug() << ind_send << s_data->heights.at(ind_send);
//  QCPItemLine *line = new QCPItemLine(cp);
//  line->start->setCoords(0, 117.49);
//  line->end->setCoords(ind_send, s_data->heights.at(ind_send));

  auto [x, p] = (min_height_send.second < min_height_rec.second)
                    ? std::make_pair(ind_send, min_height_send)
                    : std::make_pair(ind_rec, min_height_rec);

  qreal delta_y = qAbs(s_data->heights.at(x) - p.second);
  qreal a =
      obstacleSphereRadius(findlNull(int_start, int_end, p) * dist, delta_y);
  qreal s = distanceSquare(a);
  //  qDebug() << s;
  //  qDebug() << relativeDistances(s, min_height_send.first * dist);
  //  qDebug() << relativeDistances(
  //      s, qAbs(s_data->heights.size() - min_height_rec.first - 1));
}

// y = ax + b
std::pair<qreal, qreal> ClosedInterval::strLineEquation(qreal x, qreal y,
                                                        qreal x_relief,
                                                        qreal y_relief) const {
  qreal c = (y_relief - y);
  qreal d = (x_relief - x);
  return {c / d, y - (x * c / d)};
}

bool ClosedInterval::isTangent(qint32 int_start, qint32 int_end, qreal a,
                               qreal b) const {
  for (auto j = int_start; j <= int_end; ++j) {
    if (a * j * s_data->intervals_difference + b < s_data->heights.at(j))
      return false;
  }
  return true;
}

std::pair<qint32, qreal> ClosedInterval::findMinHeight(qreal a, qreal b,
                                                       qint32 start,
                                                       qint32 end) {
  auto min_height =
      std::max_element(s_data->heights.begin(), s_data->heights.end());
  qint32 height_index = 0;
  QVector<qreal> x, y;
  for (qint32 i = start, k = 0; i <= end; ++i, ++k) {
    x.push_back(i * s_data->intervals_difference);
    y.push_back(a * i * s_data->intervals_difference + b -
                qAbs(s_data->H_null.at(i)));

    if (y.at(k) < s_data->heights.at(i) + 1 &&
        y.at(k) > s_data->heights.at(i) - 1) {
      *min_height = std::min(*min_height, s_data->heights.at(i));
      height_index = i;
    }
  }
  //  if (!height_index) {

  //  }
  return {height_index, *min_height};
}

qreal findlNull(qint32 start, qint32 end,
                std::pair<qint32, qreal> p) {  // TODO: нужен больший обзор
  auto it = std::find_if(
      s_data->heights.begin() + start, s_data->heights.begin() + end,
      [=](auto val) { return (val + 1 > p.second) && (val - 1 < p.second); });
  return qAbs(std::distance(it, s_data->heights.begin() + p.first));
}
