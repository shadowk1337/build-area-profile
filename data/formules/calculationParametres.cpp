#include "calculationParametres.h"
#include <QDebug>
#include "constants.h"
#include "datastruct.h"
#include "headings.h"

extern struct Data *s_data;

qreal attentuationInAtmosphere(qreal temperature) {
  qreal freq = 1.5;
  qreal ro = 7.5;
  qreal gamma_oxygen = (6.09 / (freq * freq + 0.227) + 7.19e-3 +
                        4.81 / ((freq - 57) * (freq - 57) + 1.5)) *
                       freq * freq * 10e-3;
  qreal gamma_water =
      (0.05 + 3.6 / ((freq - 22.2) * (freq - 22.2) + 8, 5) + 0.0021 * ro +
       10.6 / ((freq - 188.3) * (freq - 188.3) + 9) +
       8.9 / ((freq - 325.4) * (freq - 325.4) + 26.3)) *
      freq * freq * ro * 10e-4;
  qreal attentuationInAtmosphere =
      constants::AREA_LENGTH * (1 - (temperature - 15) * 0.01) * gamma_oxygen +
      (1 - (temperature - 15) * 0.06) * gamma_water;
  return attentuationInAtmosphere;
}

qreal attentuationInFeeders(qreal gamma, qreal hA, qreal Lr) {
  return (hA + Lr) * gamma;
}

qreal signalMidLevel()
