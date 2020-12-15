#include "calcformulas.h"
#include "constants.h"
#include "datastruct.h"
#include "intervals.h"

extern struct Data data_usage;

HalfOpenedInterval::HalfOpenedInterval() : Interval() {}

HalfOpenedInterval::HalfOpenedInterval(const QVector<qint32> &v)
    : Interval(v) {}

inline qreal HalfOpenedInterval::obstacleSphereRadius(qreal l0, qreal delta_y) {
  return ((l0 * l0) / (8 * delta_y)) * 0.001;
}

inline qreal HalfOpenedInterval::areaReliefParameter(
    qreal k, qreal H0, qreal obstacleShereRadius) {
  return pow(qPow(constants::AREA_LENGTH, 2) * k * k * ((1 - k) * (1 - k)) /
                 (obstacleShereRadius * H0),
             1.0 / 3);
}

inline qreal HalfOpenedInterval::reliefParFuncSph(qreal mu) {
  return 4 + 10 / (mu - 0.1);
}

inline qreal HalfOpenedInterval::attenuationPSph(qreal mu) {
  return 6 + 16.4 / (mu * (1 + 0.8 * mu));
}

inline qreal HalfOpenedInterval::nuWedg(qreal H, qreal k) {
  return -H *
         qSqrt(2 / (constants::LAMBDA * constants::AREA_LENGTH * k * (1 - k)));
}

inline qreal HalfOpenedInterval::attentuationPWedg(qreal nu) {
  return 6.9 + 20 * log10(qSqrt(pow(nu - 0.1, 2) + 1) + nu - 0.1);
}

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
          (idx_interval_end - idx_interval_start) *
              data_usage.s_intervals_difference,
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
  qreal k_avg = k_from_R(idx_avg * data_usage.s_intervals_difference);
  qreal mu = areaReliefParameter(k_avg, data_usage.s_HNull.at(idx_avg),
                                 obst_sph_radius);
  return {reliefParFuncSph(mu), attenuationPSph(mu)};
}

qreal HalfOpenedInterval::halfopenedIntervalWedgeApproximation(qint32 idx_avg) {
  qreal k_avg = k_from_R(idx_avg * data_usage.s_intervals_difference);
  qreal nu = nuWedg(data_usage.s_H.at(idx_avg), k_avg);
  return attentuationPWedg(nu);
}

qreal HalfOpenedInterval::findDGDots(
    qint32 first,
    qint32 last) {  // TODO: сделать считывание из файла
  QVector<qreal> x(data_usage.s_counter), y(data_usage.s_counter);
  qreal x_start = 0;
  qreal y_start = 117.49;
  qreal x_end = 33700;
  qreal y_end = 52.7;
  qreal x_diff = (x_end - x_start) / data_usage.s_counter;
  qreal y_diff = (y_end - y_start) / data_usage.s_counter;

  for (qint32 i = 0; i < data_usage.s_counter; i++) {
    x[i] = x_start + i * x_diff;
    y[i] = y_start + i * y_diff;
  }

  QVector<qint32> intersec_heights;
  for (qint32 i = 0; i < data_usage.s_counter; ++i) {
    auto a = static_cast<qint32>(data_usage.s_heights.at(i));
    auto b = static_cast<qint32>(y.at(i) - data_usage.s_HNull.at(i));
    if (a == b || a + 1 == b || b + 1 == a) intersec_heights.push_back(i);
  }
  if (intersec_heights.size() == 1 || !intersec_heights.size()) {
    qDebug() << "Not enough intersections";
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
  return abs(data_usage.s_heights.at(right) - data_usage.s_heights.at(left));
}
