/*#include "calcformules.h"
#include "constants.h"
#include "datastruct.h"
//#include "intervals.h"

using namespace Halfopened;

extern struct Data *s_data;
extern struct SenRecCoords *s_tower_coords;

HalfOpenedInterval::HalfOpenedInterval() : Interval() {}

HalfOpenedInterval::HalfOpenedInterval(const QVector<qint32> &v)
    : Interval(v) {}

void HalfOpenedInterval::IntervalType(QCustomPlot *cp,
                                      const QVector<qint32> &interval_type) {
  qint32 idx_interval_start, idx_interval_end, prev;
  idx_interval_start = prev = interval_type[0];
  QVector<qint32> v = interval_type;

  if (!uniteObstacles(v))
    qDebug() << "Ошибка при аппроксимации полуоткрытого интервала";

  for (auto it : interval_type) {
    if (it - prev > 3 ||
        it == *(interval_type.end() -
                1)) { // допускаем наличие интервалов другого типа между
                      // интервалами полуоткрытого типа
      idx_interval_end = prev;

      qreal a =
          obstacleSphereRadius((idx_interval_end - idx_interval_start) *
                                   s_data->intervals_difference,
                               deltaY(idx_interval_start, idx_interval_end));
      qint32 idx_avg = (idx_interval_end + idx_interval_start) / 2;
      if (a >=
          qSqrt(constants::AREA_LENGTH * constants::LAMBDA * 0.5 * (0.5 / 3))) {
        halfopenedIntervalSphereApproximation(idx_avg, a);
      } else {
        halfopenedIntervalWedgeApproximation(idx_avg);
      }
      idx_interval_start = it;
    }
    prev = it;
  }
}

// Аппроксимация сферой
std::pair<qreal, qreal>
HalfOpenedInterval::halfopenedIntervalSphereApproximation(
    qint32 idx_avg, qreal obst_sph_radius) {
  qreal k_avg = k(idx_avg * s_data->intervals_difference);
  qreal mu =
      areaReliefParameter(k_avg, s_data->H_null.at(idx_avg), obst_sph_radius);
  return {reliefParFuncSph(mu), attenuationPSph(mu)};
}

// Аппроксимация клином
qreal HalfOpenedInterval::halfopenedIntervalWedgeApproximation(qint32 idx_avg) {
  qreal k_avg = k(idx_avg * s_data->intervals_difference);
  qreal nu = nuWedg(s_data->H.at(idx_avg), k_avg);
  return attentuationPWedg(nu);
}

// Точки Г и Д
qreal HalfOpenedInterval::deltaY(qint32 first, qint32 last) {
  QVector<qint32> intersec_heights;

  for (auto ind : s_data->indexes) {
    auto a = static_cast<qint32>(s_data->heights.at(ind));
    auto b = static_cast<qint32>(s_tower_coords->y_sender +
                                 ind * s_tower_coords->y_diff -
                                 s_data->H_null.at(ind));
    if (b >= a - 1 && b <= a + 1)
      intersec_heights.push_back(ind);
  }

  if (intersec_heights.size() == 1 || !intersec_heights.size()) {
    return 0;
  }

  auto right = *std::lower_bound(intersec_heights.begin(),
                                 intersec_heights.end(), first);
  auto left =
      *std::lower_bound(intersec_heights.rbegin(), intersec_heights.rend(),
                        last, [](qint32 a, qint32 b) { return a > b; });
  if (right == left)
    right = *std::lower_bound(intersec_heights.begin(), intersec_heights.end(),
                              right);
  return abs(s_data->heights.at(right) - s_data->heights.at(left));
}

// Аппроксимация одним эквивалентом
bool HalfOpenedInterval::uniteObstacles(QVector<qint32> &v) {
  qint32 idx_interval_start, idx_interval_end, prev, prev_next, i,
      idx_next_start;
  idx_interval_start = prev = v[0];
  for (auto it : v) {
    if (it - prev > 3 || it == *(v.end() - 1)) {
      idx_interval_end = prev;
      idx_next_start = i = prev_next = it;
      while (++i - prev_next < 4 && i < *(v.end() - 1))
        prev_next = i;
      if (i == *(v.end() + 1) || i - idx_interval_start < 2)
        return false;
      auto a_kink =
          std::max_element(s_data->heights.begin() + idx_interval_start,
                           s_data->heights.begin() + idx_interval_end);
      auto b_kink = std::max_element(s_data->heights.begin() + idx_next_start,
                                     s_data->heights.begin() + i - 1);
      auto a = qAbs(std::distance(a_kink, s_data->heights.begin()));
      auto b = qAbs(std::distance(b_kink, s_data->heights.begin()));
      if (uniteApprox(a * s_data->intervals_difference,
                      b * s_data->intervals_difference) > 0.408) {
        v.erase(v.begin() + idx_interval_start, v.begin() + idx_interval_end);
        v.erase(v.begin() + idx_next_start, v.begin() + i);
      }
      idx_interval_start = it;
    }
    prev = it;
  }
  return true;
}*/
