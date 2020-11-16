#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qcustomplot.h>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QRegExp>
#include <cmath>
#include <cinttypes>
#include <cassert>
#include <iostream>

const double REAL_RADIUS = 6.37e+06; // действительный радиус Земли (в метрах)
const double G_STANDARD = -8e-08; // вертикальный градиент индекса преломления в приземной части тропосферы
const double R_EVALUATED = 16.9e+03; // предельное удаление края дуги от центра (в метрах)

class DataSaver{
public:
    DataSaver(qint32 names_count_ = 0) : names_count(names_count_){
        y_from_x = QMap<double, double> ();
        heights = QVector<double> ();
    }
    QMap<double, double>& GetMap(){
        return y_from_x;
    }
    void SetMap(const QMap<double, double>& m){
        y_from_x = QMap<double, double>(m);
    }
    qint32 GetCount(){
        return names_count;
    }
    void SetCount(qint32 names_count_){
        names_count = names_count_;
    }
    QVector<double>& GetHeightsArr(){
        return heights;
    }
    void SetHeightsArr(const QVector<double>& v){
        heights = QVector<double>(v);
    }
private:
    QMap<double, double> y_from_x;
    qint32 names_count;
    QVector<double> heights;
};

DataSaver* setupArc(QCustomPlot *arc, DataSaver *ds);
void setupCurve(QCustomPlot *curvPlot, DataSaver *ds);
void setupAxis(QCustomPlot *axis);
DataSaver* setupFile(QCustomPlot *f);

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    DataSaver *ds_main;
    setWindowTitle("Area profile");
    resize(1550, 980);
    move(100, 100);
    ds_main = setupFile(ui->customPlot);
    setupArc(ui->customPlot, ds_main);
    setupCurve(ui->customPlot, ds_main);
    setupAxis(ui->customPlot);

    textItem = new QCPItemText(ui->customPlot);
    connect(ui->customPlot, &QCustomPlot::mouseMove, this, &MainWindow::onMouseMove);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onMouseMove(QMouseEvent *event)
{
    QCustomPlot* customPlot = qobject_cast<QCustomPlot*>(sender());
    double x = customPlot->xAxis->pixelToCoord(event->pos().x());
    double y = customPlot->yAxis->pixelToCoord(event->pos().y());
    textItem->setText(QString("(%1, %2)").arg(static_cast<int>(x) / 1000).arg(static_cast<int>(y)));
    textItem->position->setCoords(QPointF(x, y));
    customPlot->replot();
}

// чтение данных из файла
DataSaver* setupFile(QCustomPlot *f){
    QVector<double> heights;
    DataSaver *ds = new DataSaver;
    QFile file("heights.csv");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "Could't open the data file\n";
        exit(EXIT_FAILURE);
    }
    QTextStream in(&file);
    QString test;
    QRegExp rx("[ ;]");
    qint32 count = 0;
    bool first = true;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (first){
            first = false;
            continue;
        }
        QStringList list = line.split(rx, QString::SkipEmptyParts);
        line = list[3];
        line.replace(",", ".");
        heights.push_back(line.toDouble());
        count++;
    }
    file.close();
    if (!count)
        qDebug() << "File is empty";
    ds->SetCount(count);
    ds->SetHeightsArr(heights);
    return ds;
}

// настройки осей
void setupAxis(QCustomPlot *axis){
    axis->rescaleAxes();

    axis->yAxis->scaleRange(12);
    axis->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    QString str;
    for (double i = 0; i < 2 * R_EVALUATED + 500; i += 500){
        str = QString::number(static_cast<int>(i) / 1000);
        textTicker->addTick(i, str);
    }
    axis->xAxis->setTicker(textTicker);
    axis->replot();
}


// кривая земной поверхности
double EquivalentRadius(double g);
double DeltaY(double r, double eq_radius);

DataSaver* setupArc(QCustomPlot *arcPlot, DataSaver *ds){
    QVector<double> x(2 * R_EVALUATED), y(2 * R_EVALUATED);
    QMap<double, double> m;
    double equivalent_radius = EquivalentRadius(G_STANDARD);
    bool first = true;
    double move_graph_up_value;
    double iteration_difference = 2 * R_EVALUATED / 338;
    for (double i = -R_EVALUATED, iterator = 0; i < R_EVALUATED; i += iteration_difference, iterator += iteration_difference){
        x[i + R_EVALUATED] = i + R_EVALUATED;
        if (first){
            move_graph_up_value = abs(DeltaY(i, equivalent_radius));
            y[i + R_EVALUATED] = 0;
            first = false;
            m[iterator] = y[i + R_EVALUATED];
        }
        else{
            y[i + R_EVALUATED] = -DeltaY(i, equivalent_radius) + move_graph_up_value;
            m[iterator] = y[i + R_EVALUATED];
        }
    }
    ds->SetMap(m);
    arcPlot->addGraph();
    arcPlot->graph(0)->setData(x, y);
    arcPlot->graph(0)->setPen(QPen(QColor("#014506")));
//    arcPlot->graph(0)-
    QCPItemTracer *tracer = new QCPItemTracer(arcPlot);
    tracer->setGraph(arcPlot->graph(0));
    tracer->updatePosition();
    assert(x.size() == y.size());
    return ds;
}

// кривая высот
void setupCurve(QCustomPlot *curvPlot, DataSaver *ds){
    QVector<double> heights = ds->GetHeightsArr();
    qint32 count = ds->GetCount();
    QMap<double, double> y_from_x = ds->GetMap();
    curvPlot->addGraph();
    double iteration_difference = 2 * R_EVALUATED / count;

    for (double i = 0, v = 0; v < 338; i += iteration_difference, v++)
        heights[v] += y_from_x[i];
    QCPItemLine *line;
    for (double i = 0, v = 0; v + 1 < 338; i += iteration_difference, v++){
        line = new QCPItemLine(curvPlot);
        line->setPen(QPen(QColor("#137ea8")));
        line->start->setCoords(i, heights[v]);
        line->end->setCoords(i + iteration_difference, heights[v + 1]);
        line->setClipToAxisRect(false);
    }
}

double EquivalentRadius(double g){
    return REAL_RADIUS / (1 + g * REAL_RADIUS / 2);
}

double DeltaY(double r, double eq_radius){
    return (r * r) / (2 * eq_radius);
}
