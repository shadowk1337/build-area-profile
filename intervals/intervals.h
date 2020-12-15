#ifndef INTERVALS_H
#define INTERVALS_H
#include "headings.h"

class Interval {
 public:
  Interval();
  Interval(const QVector<qint32> &);
  Interval &operator=(const Interval &);
  const QVector<qint32> &getIntType(void) const;
  virtual void IntervalType(QCustomPlot *, const QVector<qint32> &) = 0;

 protected:
  QVector<qint32> m_interval_type;
};

class OpenedInterval : public Interval {
public:
  OpenedInterval();
  OpenedInterval(const QVector<qint32> &);
  void IntervalType(QCustomPlot *, const QVector<qint32> &) override;

private:
  void openedIntervalPlaneApproximation(qint32, qint32, qint32);
  void openedIntervalSphereApproximation(qint32, qint32, qint32);
  void rayleighAndGroundCriteria(qint32 line_start, qint32 line_end,
                                 qint32 idx_line_mid);
};

class HalfOpenedInterval : public Interval {
 public:
  HalfOpenedInterval();
  HalfOpenedInterval(const QVector<qint32> &);
  void IntervalType(QCustomPlot *, const QVector<qint32> &) override;

 private:
  qreal findDGDots(qint32, qint32);
  std::pair<qreal, qreal> halfopenedIntervalSphereApproximation(qint32, qreal);
  qreal halfopenedIntervalWedgeApproximation(qint32);
  inline qreal obstacleSphereRadius(qreal l0, qreal delta_y);
  inline qreal areaReliefParameter(qreal k, qreal H0,
                                   qreal obstacleShereRadius);
  inline qreal reliefParFuncSph(qreal mu);
  inline qreal attenuationPSph(qreal mu);
  inline qreal nuWedg(qreal H, qreal k);
  inline qreal attentuationPWedg(qreal nu);
};

class ClosedInterval : public Interval {
 public:
  ClosedInterval();
  ClosedInterval(const QVector<qint32> &);
  void IntervalType(QCustomPlot *, const QVector<qint32> &) override;

 private:
  qint32 findLongestInterval();
  void reliefTangentStraightLines(QCustomPlot *, qint32, qint32);
  inline std::pair<qreal, qreal> strLineEquation(qreal, qreal, qreal, qreal);
  inline bool checkTangentLine(qint32, qint32, qreal, qreal);
};

#endif  // INTERVALS_H
