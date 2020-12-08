#ifndef USEFULDATA_H
#define USEFULDATA_H
#include <QVector>
#include <QMap>
#include <cassert>

namespace UsefulData {
template <typename K, typename V>
class Map {
 public:
  Map();
  explicit Map(const QMap<K, V>&);
  void setMap(const QMap<K, V>&);
  QMap<K, V> getMap(void) const;
  V& operator[](K);
 private:
  QMap<qreal, qreal> m_map;
};

template <typename T>
class Vector {
 public:
  Vector();
  explicit Vector(const QVector<T>&);
  void setVector(const QVector<T>&);
  QVector<T> getVector(void) const;
  T& operator[](qint32);

 private:
  QVector<T> m_vector;
};
}  // namespace UsefulData

template <typename K, typename V>
UsefulData::Map<K, V>::Map() {}

template <typename K, typename V>
UsefulData::Map<K, V>::Map(const QMap<K, V> &m) {
  m_map = m;
  assert(m_map.size() == m.size());
}

template <typename K, typename V>
void UsefulData::Map<K, V>::setMap(const QMap<K, V> &m) {
  m_map = m;
  assert(m_map.size() == m.size());
}

template <typename K, typename V>
QMap<K, V> UsefulData::Map<K, V>::getMap(void) const {
  return m_map;
}

template <typename K, typename V>
V& UsefulData::Map<K, V>::operator[](K idx){
    return m_map[idx];
}

template <typename T>
UsefulData::Vector<T>::Vector() {
  m_vector.resize(0);
}

template <typename T>
UsefulData::Vector<T>::Vector(const QVector<T> &v) {
  m_vector = v;
  assert(m_vector.size() == v.size());
}

template <typename T>
void UsefulData::Vector<T>::setVector(const QVector<T> &v) {
  m_vector = v;
  assert(m_vector.size() == v.size());
}

template <typename T>
QVector<T> UsefulData::Vector<T>::getVector(void) const {
  return m_vector;
}

template <typename T>
T& UsefulData::Vector<T>::operator[](qint32 idx){
    return m_vector[idx];
}
#endif  // USEFULDATA_H
