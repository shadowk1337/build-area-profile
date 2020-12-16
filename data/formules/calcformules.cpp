#include "calcformules.h"
#include "constants.h"
#include "datastruct.h"
#include "headings.h"

extern struct Data s_data;

qreal k(qreal R) { return R / constants::AREA_LENGTH; }

qreal equivalentRadius(qreal g) {
  return constants::REAL_RADIUS / (1 + g * constants::REAL_RADIUS / 2);
}

qreal deltaY(qreal r, qreal eq_radius) { return (r * r) / (2 * eq_radius); }

qreal lNull(qreal h0, qreal k) {
  return ((constants::AREA_LENGTH)*qSqrt(1 + h0 * h0)) /
         (1 + (h0 * h0) / (4 * k * (1 - k)));
}

qreal HNull(qint32 i) {
  return sqrt(constants::AREA_LENGTH * constants::LAMBDA *
               k(i * s_data.intervals_difference) *
               ((1 - k(i * s_data.intervals_difference)) / 3));
}
