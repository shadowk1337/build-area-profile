#ifndef INTERVALS_H
#define INTERVALS_H
#include "headings.h"

class Interval {
 public:
  Interval();
  //  Interval(const QVector<qint32> &);
  //  const QVector<qint32> &getIntType(void) const;
  //  virtual void IntervalType(QCustomPlot *, const QVector<qint32> &) = 0;

  // protected:
  //  QVector<qint32> m_interval_type;
};

/*class OpenedInterval : public Interval {
 public:
  OpenedInterval();
  OpenedInterval(const QVector<qint32> &);
  void IntervalType(QCustomPlot *, const QVector<qint32> &) override;

 private:
  void openedIntervalPlaneApproximation(qint32, qint32);
  void openedIntervalSphereApproximation(qint32, qint32);
  void rayleighAndGroundCriteria(qint32 line_start, qint32 line_end);
};*/

/*class HalfOpenedInterval : public Interval {
 public:
  HalfOpenedInterval();
  HalfOpenedInterval(const QVector<qint32> &);
  void IntervalType(QCustomPlot *, const QVector<qint32> &) override;

 private:
  qreal deltaY(qint32, qint32);
  std::pair<qreal, qreal> halfopenedIntervalSphereApproximation(qint32, qreal);
  qreal halfopenedIntervalWedgeApproximation(qint32);
  bool uniteObstacles(QVector<qint32> &);
};*/

class ClosedInterval {
 public:
  ClosedInterval();
  //  ClosedInterval(const QVector<qint32> &);
  ClosedInterval(QCustomPlot *);

 private:
  qint32 countPeaks(void);
  std::pair<qint32, qreal> findMinHeight(qreal, qreal, qint32, qint32);
  qint32 findLongestInterval() const;
  void reliefTangentStraightLines(QCustomPlot *, qint32, qint32);
  inline std::pair<qreal, qreal> strLineEquation(qreal, qreal, qreal,
                                                 qreal) const;
  inline bool isTangent(qint32, qint32, qreal, qreal) const;
};

#endif  // INTERVALS_H
