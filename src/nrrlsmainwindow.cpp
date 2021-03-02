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
  //  _d->ui->customPlot->close();

  setSettings(options);

  setWindowTitle(tr("РРЛС"));
  int w = 1360;
  int h = 768;
  const QRect &r = qApp->desktop()->screen()->rect();
  setGeometry(qAbs(r.width() - w) / 2, qAbs(r.height() - h) / 2, w, h);

  _d->ui->progressBar->hide();

  QWidget *a = new QWidget(this);
  a->setGeometry(2, 695, 195, 50);
  la = new QHBoxLayout(a);
  pushButton_fileDial = new QPushButton("Файл", this);
  a->setLayout(la);
  la->addWidget(pushButton_fileDial);
  pushButton_fileDial->setMaximumSize(a->width(), a->height());

  connect(pushButton_fileDial, &QPushButton::clicked, [&]() {
    QFileDialog *in = new QFileDialog(this, "Open File", "*.csv");
    in->setOption(QFileDialog::DontUseNativeDialog, QFileDialog::ReadOnly);
    QString temp = in->getOpenFileName();
    in->hide();
    _d->_c = QSharedPointer<NRrls::Calc::Core>::create(_d->ui, temp);
    init();
  });

  connect(_d->ui->lineEdit_freq, &QLineEdit::editingFinished, [&]() {
    QString temp = _d->ui->lineEdit_freq->text();
    _d->_c->setFreq(temp.toDouble());
    init();
  });

  connect(_d->ui->horizontalSlider_sender, &QSlider::valueChanged, [&]() {
    int temp = _d->ui->horizontalSlider_sender->value();
    _d->ui->label_sender->setText(QString::number(temp));
  });

  connect(_d->ui->horizontalSlider_sender, &QSlider::sliderReleased, [&]() {
    int temp = _d->ui->horizontalSlider_sender->value();
    _d->_c->setSenHeight(temp);
    if (_d->ui->customPlot->graphCount() >= 5) {
      _d->ui->customPlot->graph(2)->data().data()->clear();
      _d->ui->customPlot->graph(3)->data().data()->clear();
      _d->ui->customPlot->graph(4)->data().data()->clear();
    }
    init();
  });

  connect(_d->ui->horizontalSlider_reciever, &QSlider::valueChanged, [&]() {
    int temp = _d->ui->horizontalSlider_reciever->value();
    _d->_c->setSenHeight(temp);
    _d->ui->lineEdit_reciever->setText(QString::number(temp));
  });


  connect(_d->ui->horizontalSlider_reciever, &QSlider::sliderReleased, [&]() {
    int temp = _d->ui->horizontalSlider_reciever->value();
    _d->_c->setRecHeight(temp);
    if (_d->ui->customPlot->graphCount() >= 5) {
      _d->ui->customPlot->graph(2)->data().data()->clear();
      _d->ui->customPlot->graph(3)->data().data()->clear();
      _d->ui->customPlot->graph(4)->data().data()->clear();
    }
    init();
  });

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
