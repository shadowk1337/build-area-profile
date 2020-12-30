#include <algorithm>
#include "calcformules.h"
#include "constants.h"
#include "datastruct.h"
#include "intervals.h"

using namespace Opened;

extern struct Data *s_data;
extern struct SenRecCoords *s_tower_coords;

OpenedInterval::OpenedInterval() : Interval() {}

void OpenedInterval::exec() {
  findPointOfIntersection(*s_data->indexes.begin(),
                          *(s_data->indexes.end() - 1));
}

std::pair<qint32, qint32> lineOfSightCoords(qint32, qint32);
qreal lNull_mid(qint32, qint32);

// Нахождение точки пересечения прямой А'Б с кривой поверхности
void OpenedInterval::findPointOfIntersection(qint32 int_start, qint32 int_end) {
  qreal opposite_y_coord =
      2 * s_data->heights.at(int_start) - s_tower_coords->y_sender;
  auto [a, b] = strLineEquation(int_start, opposite_y_coord,
                                int_end * s_data->intervals_difference,
                                s_tower_coords->y_reciever);
  qreal intersection_index = -1;
  qreal max_height = 0;
  qreal l_null_length;
  for (auto it = int_start; it < int_end;
       ++it) {  // поиск точки пересечения, если их несколько, то берется
                // наивысшая точка
    auto y_coord = a * it * s_data->intervals_difference + b;
    if ((y_coord >= s_data->heights.at(it) &&
         y_coord <= s_data->heights.at(it + 1)) ||
        (y_coord <= s_data->heights.at(it) &&
         y_coord >= s_data->heights.at(it + 1))) {
      if (y_coord > max_height) {
        max_height = y_coord;
        intersection_index = it;
      }
    }
  }
  if (intersection_index == -1) {
    qDebug() << "Bad opened interval. Terminating.";
    exit(EXIT_FAILURE);
  }
  if ((l_null_length =
          lNull(s_data->h_null.at(intersection_index),
                k(intersection_index * s_data->intervals_difference))) <=
          0.25 * constants::AREA_LENGTH)
    openedIntervalPlaneApproximation(
        std::max(int_start, static_cast<qint32>(
                                intersection_index -
                                l_null_length / s_data->intervals_difference)),
        std::min(int_end, static_cast<qint32>(
                              intersection_index +
                              l_null_length / s_data->intervals_difference)));
  else
    openedIntervalSphereApproximation(
        (intersection_index - l_null_length / s_data->intervals_difference),
        std::min(int_end, static_cast<qint32>(
                              intersection_index +
                              l_null_length / s_data->intervals_difference)));
}

// Открытые интервалы
void OpenedInterval::openedIntervalPlaneApproximation(qint32 int_start,
                                                      qint32 int_end) {
  rayleighAndGroundCriteria(int_start, int_end);
}

void OpenedInterval::openedIntervalSphereApproximation(qint32 int_start,
                                                       qint32 int_end) {
  auto h = s_data->heights;
  auto min_height =
      *std::min_element(h.begin() + int_start, h.begin() + int_end);
  auto max_height =
      *std::max_element(h.begin() + int_start, h.begin() + int_end);
  auto delta_y = qAbs(max_height - min_height);
  if (max_height == min_height) return;
  //  qDebug() << obstacleSphereRadius(
  //      (int_end - int_start) * s_data->intervals_difference, delta_y);
}

void OpenedInterval::rayleighAndGroundCriteria(qint32 int_start,
                                               qint32 int_end) {
  qreal k, b, denominator, y, delta_h;
  auto h = s_data->heights;
  auto [a, z] = lineOfSightCoords(int_start, int_end);
  denominator = (z - a);
  k = (qreal(h.at(z) - h.at(a)) / denominator);
  b = h.at(a) - (a * (h.at(z) - h.at(a))) / denominator;
  auto max_H0_h0 =
      *std::max_element(s_data->HNull_hNull_div.begin() + int_start,
                        s_data->HNull_hNull_div.begin() + int_end);
  for (auto i = int_start; i <= int_end; ++i) {
    y = k * i + b;
    delta_h = abs(y - h.at(i));
    //        qDebug() << delta_h << 0.75 * max_H0_h0;
    //    if (delta_h < 0.75 * max_H0_h0) {
    //      qDebug() << i << ": Гладкая";
    //      qDebug() << attentuationPlane(delta_h);
    //    } else
    //      qDebug() << '-';
  }
}

qreal findEl(qint32 piv, qint32 limit) {
  auto h = s_data->heights;
  auto a = qAbs(piv - limit);
  for (qint32 i = 0; i < a; ++i) {
    qint32 ind;
    if (limit > piv)
      ind = i + piv;
    else
      ind = piv - i;
    if (h.at(ind - 1) <= h.at(ind) && h.at(ind + 1) <= h.at(ind)) {
      return ind;
    }
  }
}

std::pair<qint32, qint32> lineOfSightCoords(qint32 int_start, qint32 int_end) {
  auto h = s_data->heights;
  qint32 pivot = (int_end + int_start) / 2;
  qreal max_first = int_start, max_last = int_end;
  max_last = findEl(pivot, int_end);
  max_first = findEl(pivot, int_start);
  return {max_first, max_last};
}

qreal lNull_mid(qint32 int_start, qint32 int_end) {
  auto pivot = (int_end + int_start) / 2;
  return lNull(s_data->h_null.at(pivot),
               k(pivot * s_data->intervals_difference));
  return 0;
}
