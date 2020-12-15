#include "intervals.h"

Interval::Interval() : m_interval_type(0) {}

Interval::Interval(const QVector<qint32> &v) : m_interval_type(v) {}

Interval &Interval::operator=(const Interval &inter){
  m_interval_type = inter.getIntType();
  return *this;
}

const QVector<qint32> &Interval::getIntType(void) const{
  return m_interval_type;
}
