#include <QtCore>
#include <QtWidgets>

#include "nrrlscalc.h"
#include "nrrlslogcategory.h"
#include "nrrlsmainwindow.h"
#include <iostream>

#include "ui_nrrlsmainwindow.h"

struct NRrlsMainWindow::Private {
  Private() {}

  Ui::NRrlsMainWindow *ui = nullptr;
  QSharedPointer<NRrls::Calc::Core> _c;
  QCPItemText *textItem;
};

NRrlsMainWindow::NRrlsMainWindow(const QVariantMap &options, QWidget *parent)
    : QMainWindow(parent), _d(new Private) {
  _d->ui = new Ui::NRrlsMainWindow;
  _d->ui->setupUi(this);

  setSettings(options);

  setWindowTitle(tr("РРЛС"));
  int w = 1360;
  int h = 768;
  const QRect &r = qApp->desktop()->screen()->rect();
  setGeometry(qAbs(r.width() - w) / 2, qAbs(r.height() - h) / 2, w, h);

  QWidget *a = new QWidget(this);
  la = new QHBoxLayout(this);
  l = new QLineEdit(this);
  pushButton_fileDial = new QPushButton("Файл", this);
  a->setLayout(la);
  la->addWidget(pushButton_fileDial);
  la->addWidget(l);
  a->setGeometry(2, 684, 285, 39);

  QString filename;
  connect(pushButton_fileDial, &QPushButton::clicked, [&]() {
    QString str =
        QFileDialog::getOpenFileName(a, "Open File", QString(), "*.csv ");
    l->setText(str);
//    filename = l->text();
  });
//  _d->ui->customPlot->replot();
//  _d->_c = QSharedPointer<NRrls::Calc::Core>::create(_d->ui->customPlot, filename);

  std::cout << filename.toStdString();
  connect(_d->ui->customPlot, &QCustomPlot::mouseMove, this,
          &NRrlsMainWindow::onMouseMove);
}

NRrlsMainWindow::~NRrlsMainWindow() {
  delete _d->ui;
  delete _d;
}

void NRrlsMainWindow::init() { _d->_c->exec(); }

void NRrlsMainWindow::saveSettings() {}

void NRrlsMainWindow::restoreSettings() {}

void NRrlsMainWindow::setSettings(const QVariantMap &options) {
  QSettings settings(options["config"].toString(), QSettings::IniFormat);
  // Debug level
  int level = settings.value("gui/debug_level", 1).toInt();
  setDebugLevel(options.value("level", level).toInt());
}

void NRrlsMainWindow::setWidgets() {}

void NRrlsMainWindow::setDebugLevel(int level) {
  QStringList p;
  p << "*.debug=false\n*.warning=false";

  if (level > 0) {
    p << QString("%1.debug=true\n%1.warning=true").arg("[APPL]");
  }
  if (level > 1) {
    p << QString("%1.debug=true\n%1.warning=true").arg("[%ETC]");
  }
  if (level > 2) {
    p.removeFirst();
    p.prepend("*.debug=true\n*.warning=true");
  }

  QLoggingCategory::setFilterRules(p.join("\n") + "\n");
}

void NRrlsMainWindow::onMouseMove(QMouseEvent *event) {
  QCustomPlot *customPlot = qobject_cast<QCustomPlot *>(sender());
  double x = customPlot->xAxis->pixelToCoord(event->pos().x());
  double y = customPlot->yAxis->pixelToCoord(event->pos().y());
  _d->ui->label_coords->setText(
      QString("%1, %2")
          .arg(x >= 0 ? std::round(x / 1000 * 100) / 100 : 0)
          .arg(y >= 0 ? std::round(y * 100) / 100 : 0));
  customPlot->replot();
}
