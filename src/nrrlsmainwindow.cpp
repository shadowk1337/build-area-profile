#include <QFileDialog>
#include <QtCore>
#include <QtWidgets>

#include "nrrlscalc.h"
#include "nrrlslogcategory.h"
#include "nrrlsmainwindow.h"

inline int parse(int h) { return h = (h > 100) ? 100 : (h < 0) ? 0 : h; }

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
  _d->ui->customplot_2->xAxis->setVisible(0);
  _d->ui->customplot_2->yAxis->setVisible(0);
  QCPTextElement *title = new QCPTextElement(_d->ui->customplot_2);
  title->setText(
      tr("Диаграмма уровней распределения сигнала на интервале РРС1 - РРС2"));
  title->setFont(QFont("FreeSans", 10, QFont::Bold));
  _d->ui->customplot_2->plotLayout()->insertRow(0);
  _d->ui->customplot_2->plotLayout()->addElement(0, 0, title);
  //  _d->ui->label_coords->hide();

  setWindowTitle(tr("РРЛС"));
  int w = 1360;
  int h = 768;
  const QRect &r = qApp->desktop()->screen()->rect();
  setGeometry(qAbs(r.width() - w) / 2, qAbs(r.height() - h) / 2, w, h);

  changeWidget = new QPushButton(this);
  changeWidget->setGeometry(_d->ui->stackwidget->geometry().x() +
                                _d->ui->customplot_1->geometry().x() +
                                _d->ui->customplot_1->width() - 170,
                            _d->ui->stackwidget->geometry().y() +
                                _d->ui->customplot_1->geometry().y() + 20,
                            170, 22);
  changeWidget->setText(tr("Высотный профиль"));
  changeWidget->setFont(QFont("FreeSans", 9));
  changeWidget->hide();

  _d->ui->comboBox_rrsStation1->addItem(tr("Выбрать станцию"));
  _d->ui->comboBox_rrsStation2->addItem(tr("Выбрать станцию"));
  _d->ui->comboBox_rrsStation1->addItem(tr("Р-419МЦ"));
  _d->ui->comboBox_rrsStation2->addItem(tr("Р-419МЦ"));

  connect(_d->ui->action_fileOpen, &QAction::triggered, this, [&]() {
    QFileDialog *in = new QFileDialog(this);
    in->setOption(QFileDialog::DontUseNativeDialog, QFileDialog::ReadOnly);
    QString temp = in->getOpenFileName(this, tr("Открыть файл"), "", "*.csv");
    changeWidget->show();
    if (!temp.isEmpty()) {
      _d->_c = QSharedPointer<NRrls::Calc::Core>::create(_d->ui, temp);
      //      _d->ui->label_coords->show();
      _d->ui->lineEdit_freq->setText(QString::number(1000));
      _d->ui->lineEdit_station1->setText(QString::number(20));
      _d->ui->lineEdit_station2->setText(QString::number(20));
      _d->_c->setFreq(1000);
      _d->_c->setFHeight(20);
      _d->_c->setSHeight(20);
      in->hide();
      exec();
    }
  });

  connect(_d->ui->action_fileReplot, &QAction::triggered, this, [&]() {
    exec();
    _d->ui->customplot_1->replot();
    _d->ui->customplot_2->replot();
  });

  connect(_d->ui->action_13, &QAction::triggered, this, [&]() {
    if (_d->ui->action_13->text() == "Скрыть нижнюю панель") {
      _d->ui->action_13->setText(tr("Показать нижнюю панель"));
      _d->ui->frame_hider->hide();
      _d->ui->customplot_1->resize(1341, 731);
      _d->ui->frame->resize(1341, 731);
    } else {
      _d->ui->action_13->setText(tr("Скрыть нижнюю панель"));
      _d->ui->frame_hider->show();
      _d->ui->customplot_1->resize(1341, 461);
      _d->ui->frame->resize(1341, 461);
    }
  });

  connect(_d->ui->action_4, &QAction::triggered, this,
          [&]() { QApplication::exit(); });

  connect(_d->ui->lineEdit_freq, &QLineEdit::textEdited, [&]() {
    double freq = _d->ui->lineEdit_freq->text().toDouble();
    if (_d->ui->customplot_1->graphCount() >= 5) {
      _d->_c->setFreq(freq);
      exec();
    }
  });

  connect(_d->ui->lineEdit_station1, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5) {
      double h1 = parse(_d->ui->lineEdit_station1->text().toInt());
      (h1 > 0) ? _d->ui->lineEdit_station1->setText(QString::number(h1))
               : _d->ui->lineEdit_station1->setText("0");
      _d->_c->setFHeight(h1);
      exec();
    }
  });

  connect(_d->ui->lineEdit_station2, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5) {
      double h2 = parse(_d->ui->lineEdit_station2->text().toInt());
      (h2 > 0) ? _d->ui->lineEdit_station2->setText(QString::number(h2))
               : _d->ui->lineEdit_station2->setText("0");
      _d->_c->setSHeight(h2);
      exec();
    }
  });

  //  connect(_d->ui->comboBox_rrsStation1, &QComboBox::currentTextChanged,
  //  this,
  //          &NRrlsMainWindow::onComboBoxValueChanged);

  connect(_d->ui->lineEdit_capStation1, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5) {
      _d->_c->setFPower(_d->ui->lineEdit_capStation1->text().toDouble());
    }
  });

  connect(_d->ui->lineEdit_capStation2, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5) {
      _d->_c->setSPower(_d->ui->lineEdit_capStation2->text().toDouble());
    }
  });

  connect(_d->ui->lineEdit_sensStation1, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5) {
      _d->_c->setFSensetivity(_d->ui->lineEdit_sensStation1->text().toDouble());
    }
  });

  connect(_d->ui->lineEdit_sensStation2, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5) {
      _d->_c->setSSensetivity(_d->ui->lineEdit_sensStation2->text().toDouble());
    }
  });

  connect(_d->ui->lineEdit_coefAntenna1, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5) {
      _d->_c->setFCoef(_d->ui->lineEdit_coefAntenna1->text().toDouble());
    }
  });

  connect(_d->ui->lineEdit_coefAntenna2, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5) {
      _d->_c->setSCoef(_d->ui->lineEdit_coefAntenna2->text().toDouble());
    }
  });

  connect(changeWidget, &QPushButton::clicked, [&]() {
    if (_d->ui->stackwidget->currentIndex() == 0) {
      changeWidget->setText(tr("Диаграмма уровней передачи"));
      _d->ui->stackwidget->setCurrentIndex(1);
    } else if (_d->ui->stackwidget->currentIndex() == 1) {
      changeWidget->setText(tr("Высотный профиль"));
      _d->ui->stackwidget->setCurrentIndex(0);
    }
    exec();
  });

  connect(_d->ui->lineEdit_lengthStation1, &QLineEdit::textEdited, [&]() {
    _d->_c->setFLength(_d->ui->lineEdit_lengthStation1->text().toDouble());
    exec();
  });

  connect(_d->ui->lineEdit_lengthStation2, &QLineEdit::textEdited, [&]() {
    _d->_c->setSLength(_d->ui->lineEdit_lengthStation2->text().toDouble());
    exec();
  });

  connect(_d->ui->lineEdit_temperature, &QLineEdit::textEdited, [&]() {
    _d->_c->setTemperature(_d->ui->lineEdit_temperature->text().toDouble());
    exec();
  });

  //  connect(_d->ui->customplot_1, &QCustomPlot::mouseMove, this,
  //          &NRrlsMainWindow::onMouseMove);
}

NRrlsMainWindow::~NRrlsMainWindow() {
  delete _d->ui;
  delete _d;
}

void NRrlsMainWindow::exec() { _d->_c->exec(); }

void NRrlsMainWindow::init() {}

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

// void NRrlsMainWindow::onMouseMove(QMouseEvent *event) {
//  QCustomPlot *customplot_1 = qobject_cast<QCustomPlot *>(sender());
//  double x = customplot_1->xAxis->pixelToCoord(event->pos().x());
//  double y = customplot_1->yAxis->pixelToCoord(event->pos().y());
//  _d->ui->label_coords->setText(
//      QString("%1, %2")
//          .arg(x >= 0 ? std::round(x / 1000 * 100) / 100 : 0)
//          .arg(y >= 0 ? std::round(y * 100) / 100 : 0));
//  customplot_1->replot();
//}

void NRrlsMainWindow::onComboBoxValueChanged(QComboBox *event) {
  std::cout << 'k';
  //  _d->ui->lineEdit_capStation1->setText(  );
  //  _d->ui->lineEdit_sensStation1->setText();
};

void NRrlsMainWindow::tabPressed(QEvent *event) {
  if (event->type() == QEvent::KeyPress) {
    if (static_cast<QKeyEvent *>(event)->key() == Qt::Key_Tab) return;
  }
}
