#ifndef CALCFORMULES_H
#define CALCFORMULES_H
#include <QtGlobal>

qreal k(qreal);

qreal equivalentRadius(qreal);

qreal deltaY(qreal, qreal);

qreal lNull(qreal, qreal);

qreal HNull(qint32);

qreal obstacleSphereRadius(qreal, qreal);

qreal areaReliefParameter(qreal, qreal, qreal);

qreal reliefParFuncSph(qreal);

qreal attenuationPSph(qreal);

qreal nuWedg(qreal, qreal);

qreal attentuationPWedg(qreal);

qreal distanceSquare(qreal);

qreal relativeDistances(qreal, qreal);

qreal relativePoint(qreal a, qint32 i);

qreal t(qreal, qreal);

#endif  // CALCFORMULES_H
