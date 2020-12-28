#ifndef CALCFORMULES_H
#define CALCFORMULES_H
#include <QtGlobal>

qreal k(qreal);
qreal lNull(qreal, qreal);
qreal HNull(qint32);
qreal obstacleSphereRadius(qreal, qreal);

namespace EarthApprox {
extern qreal equivalentRadius(qreal);
extern qreal deltaY(qreal, qreal);
}  // namespace EarthApprox

namespace Opened {
extern qreal attentuationPlane(qreal);
}  // namespace Opened

namespace Halfopened {
extern qreal areaReliefParameter(qreal, qreal, qreal);
extern qreal reliefParFuncSph(qreal);
extern qreal uniteApprox(qreal, qreal);
extern qreal attenuationPSph(qreal);
extern qreal nuWedg(qreal, qreal);
extern qreal attentuationPWedg(qreal);
extern qreal t(qreal, qreal);
}  // namespace Halfopened

namespace Closed {
extern qreal distanceSquare(qreal);
extern qreal relativeDistances(qreal, qreal);
extern qreal relativePoint(qreal a, qint32 i);
}  // namespace Closed

#endif  // CALCFORMULES_H
