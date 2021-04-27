#include "nrrlsgraphpainter.h"

QCustomPlot *GraphPainter::_cp;

int GraphPainter::_number;

QMap<QCustomPlot *, QMap<int, QPair<QPen, QBrush>>> GraphPainter::_data;

GraphPainter::GraphPainter(QCustomPlot *cp) {
  _cp = cp;
  _number = _data[cp].size();
}

GraphPainter::~GraphPainter() { update(_cp); }

void GraphPainter::draw(const QVector<double> &x, const QVector<double> &y, // TODO: сделать добавление в _data
                        const QString &name, QPen pen, QBrush brush) {
  _cp->addGraph();
  _cp->graph(_number)->setName(name);
  _cp->graph(_number)->setPen(pen);
  _cp->graph(_number)->setBrush(brush);
  _cp->graph(_number)->setData(x, y);
  _number++;
}

int GraphPainter::getNumber() const { return _number; }

void GraphPainter::update(QCustomPlot *cp) { 
  if (_cp == cp) {
    _number = 0;
    if (!_data[cp].empty()) _data[cp].clear();
  } else
    _data.remove(cp);
}

void GraphPainter::updateAll() {
  if (!_data.empty()) _data.clear();
}
