#include <QFileDialog>
#include <QtCore>
#include <QtWidgets>

#include "nrrlscalc.h"
#include "nrrlslogcategory.h"
#include "nrrlsmainwindow.h"

inline int heightParse(int h) { return h = (h > 100) ? 100 : (h < 0) ? 0 : h; }

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

  _d->ui->customplot_1->xAxis->setVisible(0);
  _d->ui->customplot_1->yAxis->setVisible(0);
  _d->ui->label_coords->hide();
  _d->ui->lineEdit_sender->setText("0");
  _d->ui->lineEdit_reciever->setText("0");
  _d->ui->progressBar->hide();

  setWindowTitle(tr("РРЛС"));
  int w = 1360;
  int h = 768;
  const QRect &r = qApp->desktop()->screen()->rect();
  setGeometry(qAbs(r.width() - w) / 2, qAbs(r.height() - h) / 2, w, h);

  changeWidget = new QPushButton(this);
  changeWidget->setGeometry(1130, 25, 221, 20);
  changeWidget->setText(tr("Высотный профиль"));
  changeWidget->hide();

  QWidget *a = new QWidget(_d->ui->profile);
  a->setGeometry(2, 695, 195, 50);
  la = new QHBoxLayout(a);
  fileDial = new QPushButton(tr("Файл"), _d->ui->profile);
  a->setLayout(la);
  la->addWidget(fileDial);
  fileDial->setMaximumSize(a->width(), a->height());

  int h1, h2;  ///< Высоты, задаваемые ползунками

  _d->ui->comboBox_freq->addItems({"0.1", "1.0", "3.0", "6.0", "10.0"});
  _d->ui->comboBox_freq->addItems({"15.0", "20.0", "30.0"});
  _d->ui->comboBox_freq->setCurrentText("1.0");

  connect(fileDial, &QPushButton::clicked, [&]() {
    h1 = h2 = 20;
    QFileDialog *in = new QFileDialog(this, tr("Open File"), "*.csv");
    in->setOption(QFileDialog::DontUseNativeDialog, QFileDialog::ReadOnly);
    QString temp = in->getOpenFileName();
    changeWidget->show();
    if (!temp.isEmpty()) {
      _d->ui->customplot_1->xAxis->setVisible(1);
      _d->ui->customplot_1->yAxis->setVisible(1);
      _d->ui->label_coords->show();
      _d->ui->horizontalSlider_sender->setValue(h1);
      _d->ui->horizontalSlider_reciever->setValue(h2);
      _d->ui->lineEdit_sender->setText(QString::number(h1));
      _d->ui->lineEdit_reciever->setText(QString::number(h2));
      _d->_c = QSharedPointer<NRrls::Calc::Core>::create(_d->ui, temp);
      _d->_c->setFreq(_d->ui->comboBox_freq->currentText().toDouble());
      _d->_c->setSenHeight(h1);
      _d->_c->setRecHeight(h2);
      in->hide();
      init();
    }
  });

  connect(_d->ui->comboBox_freq, &QComboBox::currentTextChanged, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5) {
      _d->_c->setFreq(_d->ui->comboBox_freq->currentText().toDouble());
      init();
    }
  });

  connect(_d->ui->horizontalSlider_sender, &QSlider::valueChanged, [&]() {
    h1 = _d->ui->horizontalSlider_sender->value();
    _d->ui->lineEdit_sender->setText(QString::number(heightParse(h1)));
  });

  connect(_d->ui->horizontalSlider_sender, &QSlider::sliderReleased, [&]() {
    h1 = _d->ui->horizontalSlider_sender->value();
    if (_d->ui->customplot_1->graphCount() >= 5) {
      _d->_c->setSenHeight(h1);
      _d->ui->customplot_1->clearGraphs();
      init();
    }
  });

  connect(_d->ui->lineEdit_sender, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5) {
      h1 = heightParse(_d->ui->lineEdit_sender->text().toInt());
      if (h1) _d->ui->lineEdit_sender->setText(QString::number(h1));
      _d->ui->horizontalSlider_sender->setValue(h1);
      _d->_c->setSenHeight(h1);
      init();
    }
  });

  connect(_d->ui->horizontalSlider_reciever, &QSlider::valueChanged, [&]() {
    h2 = _d->ui->horizontalSlider_reciever->value();
    _d->ui->lineEdit_reciever->setText(QString::number(heightParse(h2)));
  });

  connect(_d->ui->horizontalSlider_reciever, &QSlider::sliderReleased, [&]() {
    h2 = heightParse(_d->ui->horizontalSlider_reciever->value());
    if (_d->ui->customplot_1->graphCount() >= 5) {
      _d->_c->setRecHeight(h2);
      _d->ui->customplot_1->clearGraphs();
      init();
    }
  });

  connect(changeWidget, &QPushButton::clicked, [&]() {
    if (_d->ui->stackwidget->currentIndex() == 0) {
      changeWidget->setText(tr("Диаграмма уровней передачи"));
      _d->ui->stackwidget->setCurrentIndex(1);
      _d->ui->customplot_1->replot();
    } else if (_d->ui->stackwidget->currentIndex() == 1) {
      changeWidget->setText(tr("Высотный профиль"));
      _d->ui->stackwidget->setCurrentIndex(0);
    }
  });

  connect(_d->ui->lineEdit_reciever, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5) {
      h2 = heightParse(_d->ui->lineEdit_reciever->text().toInt());
      if (h2) _d->ui->lineEdit_reciever->setText(QString::number(h2));
      _d->ui->horizontalSlider_reciever->setValue(h2);
      _d->_c->setRecHeight(h2);
      init();
    }
  });

  //  connect() TODO: connect to tab

  connect(_d->ui->customplot_1, &QCustomPlot::mouseMove, this,
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
  QCustomPlot *customplot_1 = qobject_cast<QCustomPlot *>(sender());
  double x = customplot_1->xAxis->pixelToCoord(event->pos().x());
  double y = customplot_1->yAxis->pixelToCoord(event->pos().y());
  _d->ui->label_coords->setText(
      QString("%1, %2")
          .arg(x >= 0 ? std::round(x / 1000 * 100) / 100 : 0)
          .arg(y >= 0 ? std::round(y * 100) / 100 : 0));
  customplot_1->replot();
}

void NRrlsMainWindow::tabPressed(QEvent *event) {
  if (event->type() == QEvent::KeyPress) {
    if (static_cast<QKeyEvent *>(event)->key() == Qt::Key_Tab) return;
  }
}
