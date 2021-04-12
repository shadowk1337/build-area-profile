#include "graphpainter.h"

GraphPainter::GraphPainter(QCustomPlot *cp) : _cp(cp), _number(0) {
  _data[cp] = 0;
}

void GraphPainter::changePlot(QCustomPlot *cp) {
  if (!_data.contains(cp)) _data[cp] = 0;
  _cp = cp;
  _number = 0;
}

bool GraphPainter::draw(const QVector<double> &x, const QVector<double> &y,
                        QPen pen, QString name) {
  _cp->addGraph();
  _cp->graph(_number)->setPen(pen);
  _cp->graph(_number)->setName(name);
  _cp->graph(_number)->setData(x, y);
  _number++;
  return true;
}
