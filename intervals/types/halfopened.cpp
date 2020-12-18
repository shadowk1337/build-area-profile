#include "calcformules.h"
#include "constants.h"
#include "datastruct.h"
#include "intervals.h"

extern struct Data s_data;

HalfOpenedInterval::HalfOpenedInterval() : Interval() {}

HalfOpenedInterval::HalfOpenedInterval(const QVector<qint32> &v)
    : Interval(v) {}

void HalfOpenedInterval::IntervalType(
    QCustomPlot *cp,
    const QVector<qint32> &interval_type) {  // TODO: добавить аппроксимацию
                                             // двух соседних препятствий
  qint32 idx_interval_start, idx_interval_end, prev;
  idx_interval_start = prev = interval_type[0];
  QVector<qreal> attentuationP;

  for (auto it : interval_type) {
    if (it - prev > 3 ||
        it == *(interval_type.end() -
                1)) {  // допускаем наличие интервалов другого типа между
                       // интервалами полуоткрытого типа
      idx_interval_end = prev;

      qreal a = obstacleSphereRadius(
          (idx_interval_end - idx_interval_start) * s_data.intervals_difference,
          findDGDots(idx_interval_start, idx_interval_end));
      qint32 idx_avg = (idx_interval_end + idx_interval_start) / 2;
      if (a >=
          sqrt(constants::AREA_LENGTH * constants::LAMBDA * 0.5 * (0.5 / 3))) {
        halfopenedIntervalSphereApproximation(idx_avg, a);
      } else {
        halfopenedIntervalWedgeApproximation(idx_avg);
      }
      idx_interval_start = it;
    }
    prev = it;
  }
}

std::pair<qreal, qreal>
HalfOpenedInterval::halfopenedIntervalSphereApproximation(
    qint32 idx_avg, qreal obst_sph_radius) {
  qreal k_avg = k(idx_avg * s_data.intervals_difference);
  qreal mu =
      areaReliefParameter(k_avg, s_data.H_null.at(idx_avg), obst_sph_radius);
  return {reliefParFuncSph(mu), attenuationPSph(mu)};
}

qreal HalfOpenedInterval::halfopenedIntervalWedgeApproximation(qint32 idx_avg) {
  qreal k_avg = k(idx_avg * s_data.intervals_difference);
  qreal nu = nuWedg(s_data.H.at(idx_avg), k_avg);
  return attentuationPWedg(nu);
}

qreal HalfOpenedInterval::findDGDots(qint32 first, qint32 last) {
  QVector<qreal> x(s_data.counter), y(s_data.counter);
  qreal x_start = 0;
  qreal y_start = 117.49;
  qreal x_end = 33700;
  qreal y_end = 52.7;
  qreal x_diff = (x_end - x_start) / s_data.counter;
  qreal y_diff = (y_end - y_start) / s_data.counter;

  for (qint32 i = 0; i < s_data.counter; i++) {
    x[i] = x_start + i * x_diff;
    y[i] = y_start + i * y_diff;
  }

  QVector<qint32> intersec_heights;

  for (qint32 i = 0; i < s_data.counter; ++i) {
    auto a = static_cast<qint32>(s_data.heights.at(i));
    auto b = static_cast<qint32>(y.at(i) - s_data.H_null.at(i));
    if (a == b || a + 1 == b || b + 1 == a) intersec_heights.push_back(i);
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
  return abs(s_data.heights.at(right) - s_data.heights.at(left));
}
