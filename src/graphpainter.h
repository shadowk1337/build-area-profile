#ifndef GRAPHPAINTER_H
#define GRAPHPAINTER_H

#include <iostream>
#include "qcustomplot.h"

class GraphPainter {
 public:
  explicit GraphPainter(QCustomPlot *cp);

  void changePlot(QCustomPlot *cp);

  void draw(const QVector<double> &x, const QVector<double> &y, QPen pen = {},
            QString name = QObject::tr("graph"));

  void update(QCustomPlot *cp) {
    if (_cp == cp)
      _number = 0;
    else
      _data.remove(cp);
  }

  void updateAll() { _data.clear(); }

  GraphPainter() = delete;
  GraphPainter(GraphPainter const &) = delete;
  GraphPainter &operator=(GraphPainter const &) = delete;

 private:
  QCustomPlot *_cp;
  int _number;

 private:
  static QMap<QCustomPlot *, int> _data;
};

#endif  // GRAPHPAINTER_H
