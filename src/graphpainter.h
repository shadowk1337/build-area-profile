#ifndef GRAPHPAINTER_H
#define GRAPHPAINTER_H

#include "qcustomplot.h"
#include <iostream>

class GraphPainter {
 public:
  explicit GraphPainter(QCustomPlot *cp);

  void changePlot(QCustomPlot *cp);

  bool draw(const QVector<double> &x, const QVector<double> &y, QPen pen = {},
            QString name = QObject::tr("graph"));

  GraphPainter() = delete;
  GraphPainter(GraphPainter const &) = delete;
  GraphPainter &operator=(GraphPainter const &) = delete;

 private:
  QCustomPlot *_cp;
  int _number;

 private:
  QMap<QCustomPlot *, int> _data;
};

#endif  // GRAPHPAINTER_H
