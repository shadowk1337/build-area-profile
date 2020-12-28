#include "calcformules.h"
#include <QDebug>
#include "constants.h"
#include "datastruct.h"
#include "headings.h"

extern struct Data *s_data;

qreal k(qreal R) { return R / constants::AREA_LENGTH; }

qreal EarthApprox::equivalentRadius(qreal g) {
  return constants::REAL_RADIUS / (1 + g * constants::REAL_RADIUS / 2);
}

qreal EarthApprox::deltaY(qreal r, qreal eq_radius) {
  return (r * r) / (2 * eq_radius);
}

qreal lNull(qreal h0, qreal k) {
  return ((constants::AREA_LENGTH)*qSqrt(1 + h0 * h0)) /
         (1 + (h0 * h0) / (4 * k * (1 - k)));
}

qreal HNull(qint32 i) {
  return sqrt(constants::AREA_LENGTH * constants::LAMBDA *
              k(i * s_data->intervals_difference) *
              ((1 - k(i * s_data->intervals_difference)) / 3));
}

qreal Opened::attentuationPlane(qreal delta_h) {
  return -10 * log10(2 - 2 * qCos(M_PI / 3 * delta_h * delta_h));
}

qreal obstacleSphereRadius(qreal l0, qreal delta_y) {
  return ((l0 * l0) / (8 * delta_y)) * 0.001;
}

qreal Halfopened::areaReliefParameter(qreal k, qreal H0,
                                      qreal obstacleShereRadius) {
  return pow(qPow(constants::AREA_LENGTH, 2) * k * k * ((1 - k) * (1 - k)) /
                 (obstacleShereRadius * H0),
             1.0 / 3);
}

qreal Halfopened::reliefParFuncSph(qreal mu) { return 4 + 10 / (mu - 0.1); }

qreal Halfopened::attenuationPSph(qreal mu) {
  return 6 + 16.4 / (mu * (1 + 0.8 * mu));
}

qreal Halfopened::nuWedg(qreal H, qreal k) {
  return -H *
         qSqrt(2 / (constants::LAMBDA * constants::AREA_LENGTH * k * (1 - k)));
}

qreal Halfopened::attentuationPWedg(qreal nu) {
  return 6.9 + 20 * log10(qSqrt(pow(nu - 0.1, 2) + 1) + nu - 0.1);
}

qreal Closed::distanceSquare(qreal obstSphRadius) {
  return qPow((obstSphRadius * obstSphRadius * constants::LAMBDA) / M_PI,
              0.33333);
}

qreal Closed::relativeDistances(qreal s, qreal r) { return s / r; }

qreal Closed::relativePoint(qreal a, qint32 i) {
  auto z = k(i * s_data->intervals_difference);
  return (a * s_data->H.at(i)) / (constants::AREA_LENGTH * z * (1 - z));
}

qreal Halfopened::t(qreal l, qreal s) { return l / s; }

qreal Halfopened::uniteApprox(qreal R1, qreal R2) {
  return log10(M_PI - qAsin(qSqrt(constants::AREA_LENGTH * (R2 - R1) /
                                  (R2 * (constants::AREA_LENGTH - R1)))));
}
