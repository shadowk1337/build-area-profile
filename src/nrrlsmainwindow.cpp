#include <QFileDialog>
#include <QtCore>
#include <QtWidgets>

#include "nrrlscalc.h"
#include "nrrlslogcategory.h"
#include "nrrlsmainwindow.h"

struct NRrlsMainWindow::Private {
  Private() {}

  Ui::NRrlsMainWindow *ui = nullptr;
  QSharedPointer<NRrls::Calc::Core> _c;
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
  double s_w = _d->ui->horizontalSlider_sender->width();
  double s_h = _d->ui->horizontalSlider_sender->height();
  QLabel *l1 = new QLabel("0", _d->ui->horizontalSlider_sender);
  QLabel *l2 = new QLabel("25", _d->ui->horizontalSlider_sender);
  l2->move(s_w / 4, s_h);
  QLabel *l3 = new QLabel("50", _d->ui->horizontalSlider_sender);
  l3->move(s_w / 2, s_h);
  QLabel *l4 = new QLabel("75", _d->ui->horizontalSlider_sender);
  l4->move(s_w * 3 / 4, s_h);
  QLabel *l5 = new QLabel("100", _d->ui->horizontalSlider_sender);
  l5->move(s_w, s_h);

  QWidget *a = new QWidget(this);
  a->setGeometry(2, 700, 285, 50);
  la = new QHBoxLayout(a);
  pushButton_fileDial = new QPushButton("Файл", this);
  a->setLayout(la);
  la->addWidget(pushButton_fileDial);
  pushButton_fileDial->setMaximumSize(a->width(), a->height());

  connect(pushButton_fileDial, &QPushButton::clicked, [&]() {
    QString temp = QFileDialog::getOpenFileName(this, "Open File", "*.csv");
    _d->_c = QSharedPointer<NRrls::Calc::Core>::create(_d->ui, temp);
    init();
  });

  connect(_d->ui->lineEdit_freq, &QLineEdit::editingFinished, [&]() {
    QString temp = _d->ui->lineEdit_freq->text();
    _d->_c->setFreq(temp.toDouble());
    init();
  });

  //  connect(_d->ui->lineEdit_senderHeight, &QLineEdit::editingFinished, [&]()
  //  {
  //    QString temp = _d->ui->lineEdit_senderHeight->text();
  //    _d->_c->setSenHeight(temp.toDouble());
  //    init();
  //  });

  //  connect(_d->ui->lineEdit_recieverHeight, &QLineEdit::editingFinished,
  //  [&]() {
  //    QString temp = _d->ui->lineEdit_recieverHeight->text();
  //    _d->_c->setRecHeight(temp.toDouble());
  //    init();
  //  });

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
