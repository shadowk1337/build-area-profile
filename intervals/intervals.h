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
};

class ClosedInterval : public Interval {
 public:
  ClosedInterval();
  ClosedInterval(const QVector<qint32> &);
  void IntervalType(QCustomPlot *, const QVector<qint32> &) override;

 private:
  std::pair<qint32, qreal> findMinHeight(qreal, qreal, qint32, qint32);
  qint32 findLongestInterval() const;
  void reliefTangentStraightLines(qint32, qint32);
//  void lineInfo(QCustomPlot *, qint32, qint32, qreal, qreal, qint32);
  inline std::pair<qreal, qreal> strLineEquation(qreal, qreal, qreal,
                                                 qreal) const;
  inline bool checkTangentLine(qint32, qint32, qreal, qreal) const;
};

#endif  // INTERVALS_H
