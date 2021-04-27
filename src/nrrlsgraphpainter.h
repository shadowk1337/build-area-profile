#ifndef GRAPHPAINTER_H
#define GRAPHPAINTER_H

#include "qcustomplot.h"

class GraphPainter {
 public:
  using Attribute = QMap<int, QPair<QPen, QBrush>>;

  explicit GraphPainter(QCustomPlot *cp);

  ~GraphPainter();

  void draw(const QVector<double> &x, const QVector<double> &y,
            const QString &name, QPen pen = {}, QBrush brush = {});

  int getNumber() const;

  static void update(QCustomPlot *cp);

  static void updateAll();

  GraphPainter() = delete;
  GraphPainter(GraphPainter const &) = delete;
  GraphPainter &operator=(GraphPainter const &) = delete;

 private:
  static QCustomPlot *_cp;
  static int _number;

 private:
  static QMap<QCustomPlot *, Attribute> _data;
};

#endif  // GRAPHPAINTER_H
