#include "calcformules.h"
#include "constants.h"
#include "datastruct.h"
#include "intervals.h"

using namespace Halfopened;

extern struct Data *s_data;
extern struct SenRecCoords *s_tower_coords;

HalfOpenedInterval::HalfOpenedInterval() : Interval() {}

void HalfOpenedInterval::exec() {
  approx(*s_data->indexes.begin(), *(s_data->indexes.end() - 1));
}

void HalfOpenedInterval::approx(qint32 int_start, qint32 int_end) {
  qreal l_null_length;
  auto index_H = shadingObstacle();
  l_null_length = lNull(s_data->h_null.at(index_H.first),
                        k(index_H.first * s_data->intervals_difference));

//  if (!uniteObstacles(int_start, int_end)) {
    qreal a = obstacleSphereRadius(l_null_length, deltaY(int_start, int_end));
    if (a >=
        qSqrt(constants::AREA_LENGTH * constants::LAMBDA * 0.5 * (0.5 / 3))) {
      halfopenedIntervalSphereApproximation(index_H.first, a);
    } else {
      halfopenedIntervalWedgeApproximation(index_H.first);
    }
//  }
}

// Аппроксимация сферой
std::pair<qreal, qreal>
HalfOpenedInterval::halfopenedIntervalSphereApproximation(
    qint32 idx, qreal obst_sph_radius) {
  qreal k_avg = k(idx * s_data->intervals_difference);
  qreal mu =
      areaReliefParameter(k_avg, s_data->H_null.at(idx), obst_sph_radius);
  return {reliefParFuncSph(mu), attenuationPSph(mu)};
}

// Аппроксимация клином
qreal HalfOpenedInterval::halfopenedIntervalWedgeApproximation(qint32 idx) {
  qreal k_avg = k(idx * s_data->intervals_difference);
  qreal nu = nuWedg(s_data->H.at(idx), k_avg);
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
    if (b >= a - 1 && b <= a + 1) intersec_heights.push_back(ind);
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

std::pair<qint32, qreal> HalfOpenedInterval::shadingObstacle(void) const {
  auto it = std::max_element(s_data->H.begin(), s_data->H.end());
  qreal dist = std::distance(s_data->H.begin(), it);  // индекс препятствия
  return {dist, *it};
}

// Аппроксимация одним эквивалентом
/*bool HalfOpenedInterval::uniteObstacles(qint32 int_start, qint32 int_end) {
  for (auto it : v) {
    while (++i - prev_next < 4 && i < *(v.end() - 1)) prev_next = i;
    if (i == *(v.end() + 1) || i - idx_interval_start < 2) return false;
    auto a_kink = std::max_element(s_data->heights.begin() +
idx_interval_start, s_data->heights.begin() + idx_interval_end); auto b_kink =
std::max_element(s_data->heights.begin() + idx_next_start,
                                   s_data->heights.begin() + i - 1);
    auto a = qAbs(std::distance(a_kink, s_data->heights.begin()));
    auto b = qAbs(std::distance(b_kink, s_data->heights.begin()));
    if (uniteApprox(a * s_data->intervals_difference,
                    b * s_data->intervals_difference) > 0.408) {
      v.erase(v.begin() + idx_interval_start, v.begin() + idx_interval_end);
      v.erase(v.begin() + idx_next_start, v.begin() + i);
    }
    idx_interval_start = it;
    prev = it;
  }
  return true;
}*/
