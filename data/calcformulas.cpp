#include "headings.h"
#include "constants.h"

qreal k_from_R(qreal R) { return R / constants::AREA_LENGTH; }

qreal equivalentRadius(qreal g) {
  return constants::REAL_RADIUS / (1 + g * constants::REAL_RADIUS / 2);
}

qreal deltaY(qreal r, qreal eq_radius) { return (r * r) / (2 * eq_radius); }

qreal l0(qreal h0, qreal k) {
  return ((constants::AREA_LENGTH)*qSqrt(1 + h0 * h0)) /
         (1 + (h0 * h0) / (4 * k * (1 - k)));
}
