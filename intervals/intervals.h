#ifndef INTERVALS_H
#define INTERVALS_H
#include "headings.h"

class Interval {
 public:
  Interval();
  virtual void exec(void) = 0;

 protected:
  std::pair<qreal, qreal> strLineEquation(qreal, qreal, qreal, qreal) const;
};

class OpenedInterval : public Interval {
 public:
  OpenedInterval();
  void exec(void) override;

 private:
  void findPointOfIntersection(qint32, qint32);
  void openedIntervalPlaneApproximation(qint32, qint32);
  void openedIntervalSphereApproximation(qint32, qint32);
  void rayleighAndGroundCriteria(qint32 line_start, qint32 line_end);
};

class HalfOpenedInterval : public Interval {
 public:
  HalfOpenedInterval();
  void exec() override;

 private:
  void approx(qint32, qint32);
  qreal deltaY(qint32, qint32);
  std::pair<qreal, qreal> halfopenedIntervalSphereApproximation(qint32, qreal);
  qreal halfopenedIntervalWedgeApproximation(qint32);
  std::pair<qint32, qreal> shadingObstacle(void) const;
//  bool uniteObstacles(qint32, qint32);
};

class ClosedInterval : public Interval {
 public:
  ClosedInterval();
  void exec(void) override;

 private:
  qint32 countPeaks(void);
  std::pair<qint32, qreal> findMinHeight(qreal, qreal, qint32, qint32);
  qint32 findLongestInterval() const;
  void reliefTangentStraightLines(qint32, qint32);
  inline bool isTangent(qint32, qint32, qreal, qreal) const;
};

#endif  // INTERVALS_H
