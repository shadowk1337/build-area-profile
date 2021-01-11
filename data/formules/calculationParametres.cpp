#include "calculationParametres.h"
#include <QDebug>
#include <cmath>
#include "constants.h"
#include "datastruct.h"
#include "headings.h"

extern struct Data *s_data;

qreal intervalAttentuation() {  // TODO: затухания для каждого интервала
  return .0;
}

qreal attentuationAtmosphere(qreal temperature) {
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

qreal attentuationFeeders(qreal gamma, qreal hA, qreal Lr) {
  return (hA + Lr) * gamma;
}

inline qreal attentuationFreeSpace() {
  return 122 + 20 * log10(constants::AREA_LENGTH / constants::LAMBDA);
}

qreal signalMidLevel() {
  qreal power_level_send, attentuation_send, attentuation_rec, upg_coef_send,
      upg_coef_rec;
  qreal temperature;
  qreal attentuation_sum = intervalAttentuation() +
                           attentuationAtmosphere(temperature) +
                           attentuationFreeSpace();
  qreal powerLevel = power_level_send - attentuation_send + upg_coef_send -
                     attentuation_sum + upg_coef_rec - attentuation_rec;
  return powerLevel;
}

/*qreal h_req(qint32 M) {
  return 1.8 + 10 * log10(Rk * (-0.73 - log10(p)) * (M - 1) * sqrt(log2(M)));
}

qreal limitSignalRec(void) {
  qreal lim_sens_two =
      h_req + n_n - 204 + 10 * log10(B) - nu_code + tech_losses;
  qreal lim_sens =
}*/

//qreal
