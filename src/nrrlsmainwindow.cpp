#include <QFileDialog>

#include "nrrlscalc.h"
#include "nrrlslogcategory.h"
#include "nrrlsmainwindow.h"

inline int parse(int h) { return h = (h > 100) ? 100 : (h < 0) ? 0 : h; }

struct NRrlsMainWindow::Private {
  Private() {}

  Ui::NRrlsMainWindow *ui = nullptr;
  QSharedPointer<NRrls::Calc::Core> _c;
  QCPItemLine *line = nullptr, *l = nullptr;
};

double freq, h1, h2;

NRrlsMainWindow::NRrlsMainWindow(const QVariantMap &options, QWidget *parent)
    : QMainWindow(parent), _d(new Private) {
  _d->ui = new Ui::NRrlsMainWindow;
  _d->ui->setupUi(this);

  setSettings(options);
  setMouseTracking(true);

  _d->ui->customplot_1->xAxis->setVisible(0);
  _d->ui->customplot_1->yAxis->setVisible(0);
  _d->ui->customplot_2->xAxis->setVisible(0);
  _d->ui->customplot_2->yAxis->setVisible(0);
  _d->ui->customplot_3->xAxis->setVisible(0);
  _d->ui->customplot_3->yAxis->setVisible(0);

  QCPTextElement *title = new QCPTextElement(_d->ui->customplot_2);
  title->setText(
      tr("Диаграмма уровней распределения сигнала на интервале РРС1 - РРС2"));
  title->setFont(QFont("FreeSans", 10, QFont::Bold));
  _d->ui->customplot_2->plotLayout()->insertRow(0);
  _d->ui->customplot_2->plotLayout()->addElement(0, 0, title);

  title = new QCPTextElement(_d->ui->customplot_3);
  title->setText(
      tr("Диаграмма уровней распределения сигнала на интервале РРС2 - РРС1"));
  title->setFont(QFont("FreeSans", 10, QFont::Bold));
  _d->ui->customplot_3->plotLayout()->insertRow(0);
  _d->ui->customplot_3->plotLayout()->addElement(0, 0, title);

  setWindowTitle(tr("РРЛС"));
  int w = 1360;
  int h = 768;
  const QRect &r = qApp->desktop()->screen()->rect();
  setGeometry(qAbs(r.width() - w) / 2, qAbs(r.height() - h) / 2, w, h);

  _d->ui->comboBox_rrsStation1->addItem(tr("Выбрать станцию"));
  _d->ui->comboBox_rrsStation2->addItem(tr("Выбрать станцию"));
  _d->ui->comboBox_rrsStation1->addItem(tr("Р-419МЦ"));
  _d->ui->comboBox_rrsStation2->addItem(tr("Р-419МЦ"));

  _d->ui->comboBox_jobStation1->addItem(tr("Выбрать режим"));
  _d->ui->comboBox_jobStation2->addItem(tr("Выбрать режим"));

  connect(_d->ui->action_fileOpen, &QAction::triggered, this,
          &NRrlsMainWindow::setFile);

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
      _d->ui->customplot_1->resize(1341, 441);
      _d->ui->frame->resize(1341, 441);
    }
  });

  connect(_d->ui->action_4, &QAction::triggered, this,
          [&]() { QApplication::exit(); });

  connect(_d->ui->lineEdit_freq, &QLineEdit::textEdited, [&]() {
    freq = _d->ui->lineEdit_freq->text().toDouble();
    if (_d->ui->customplot_1->graphCount() >= 5) {
      _d->_c->setFreq(freq);
      exec();
    }
  });

  connect(_d->ui->lineEdit_station1, &QLineEdit::editingFinished, this,
          &NRrlsMainWindow::changeHeight);

  connect(_d->ui->lineEdit_station2, &QLineEdit::editingFinished, this,
          &NRrlsMainWindow::changeHeight);

  connect(_d->ui->comboBox_rrsStation1, &QComboBox::currentTextChanged, this,
          &NRrlsMainWindow::changeRrsSpec);

  connect(_d->ui->comboBox_rrsStation2, &QComboBox::currentTextChanged, this,
          &NRrlsMainWindow::changeRrsSpec);

  connect(_d->ui->comboBox_jobStation1, &QComboBox::currentTextChanged, this,
          &NRrlsMainWindow::changeSens);

  connect(_d->ui->comboBox_jobStation2, &QComboBox::currentTextChanged, this,
          &NRrlsMainWindow::changeSens);

  connect(_d->ui->lineEdit_capStation1, &QLineEdit::editingFinished, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5)
      _d->_c->setFPower(_d->ui->lineEdit_capStation1->text().toDouble());
  });

  connect(_d->ui->lineEdit_capStation2, &QLineEdit::editingFinished, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5)
      _d->_c->setSPower(_d->ui->lineEdit_capStation2->text().toDouble());
  });

  connect(_d->ui->lineEdit_sensStation1, &QLineEdit::editingFinished, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5)
      _d->_c->setFSensetivity(_d->ui->lineEdit_sensStation1->text().toDouble());
  });

  connect(_d->ui->lineEdit_sensStation2, &QLineEdit::editingFinished, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5)
      _d->_c->setSSensetivity(_d->ui->lineEdit_sensStation2->text().toDouble());
  });

  connect(_d->ui->lineEdit_coefAntenna1, &QLineEdit::editingFinished, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5)
      _d->_c->setFCoef(_d->ui->lineEdit_coefAntenna1->text().toDouble());
  });

  connect(_d->ui->lineEdit_coefAntenna2, &QLineEdit::editingFinished, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5)
      _d->_c->setSCoef(_d->ui->lineEdit_coefAntenna2->text().toDouble());
  });

  connect(_d->ui->lineEdit_gradient, &QLineEdit::editingFinished, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5) {
      _d->_c->setGradient(_d->ui->lineEdit_gradient->text().toDouble());
      exec();
    }
  });

  connect(_d->ui->pushButton_changeWidget, &QPushButton::clicked, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5) {
      _d->_c->setFCoef(_d->ui->lineEdit_coefAntenna1->text().toDouble());
      _d->_c->setSCoef(_d->ui->lineEdit_coefAntenna2->text().toDouble());
      _d->_c->setFFeedAtten(_d->ui->lineEdit_feederAntenna1->text().toDouble());
      _d->_c->setSFeedAtten(_d->ui->lineEdit_feederAntenna2->text().toDouble());
      _d->_c->setFPower(_d->ui->lineEdit_capStation1->text().toDouble());
      _d->_c->setSPower(_d->ui->lineEdit_capStation2->text().toDouble());
      _d->_c->setFSensetivity(_d->ui->lineEdit_sensStation1->text().toDouble());
      _d->_c->setSSensetivity(_d->ui->lineEdit_sensStation2->text().toDouble());
      _d->_c->setGradient(_d->ui->lineEdit_gradient->text().toDouble());
      _d->_c->setTemperature(_d->ui->lineEdit_temperature->text().toDouble());
      _d->ui->stackwidget->setCurrentIndex(1);
      exec();
    }
  });

  connect(_d->ui->pushButton_backWidget, &QPushButton::clicked, [&]() {
    _d->ui->stackwidget->setCurrentIndex(0);
    exec();
  });

  connect(_d->ui->lineEdit_temperature, &QLineEdit::editingFinished, [&]() {
    _d->_c->setTemperature(_d->ui->lineEdit_temperature->text().toDouble());
    exec();
  });

  connect(_d->ui->lineEdit_feederAntenna1, &QLineEdit::editingFinished, [&]() {
    _d->_c->setFFeedAtten(_d->ui->lineEdit_feederAntenna1->text().toDouble());
  });

  connect(_d->ui->lineEdit_feederAntenna2, &QLineEdit::editingFinished, [&]() {
    _d->_c->setSFeedAtten(_d->ui->lineEdit_feederAntenna2->text().toDouble());
  });

  connect(_d->ui->pushButton_addStation1, &QPushButton::clicked, this,
          &NRrlsMainWindow::openFirstStation);

  connect(_d->ui->customplot_1, &QCustomPlot::mouseDoubleClick, this,
          &NRrlsMainWindow::onCustomPlotClicked);

  connect(_d->ui->customplot_1, &QCustomPlot::mouseMove, this,
          &NRrlsMainWindow::onMouseMove);

}

NRrlsMainWindow::~NRrlsMainWindow() {
  delete _d->ui;
  delete _d->line;
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

void NRrlsMainWindow::setFile(bool checked) {
  QFileDialog *in = new QFileDialog(this);
  in->setOption(QFileDialog::DontUseNativeDialog, QFileDialog::ReadOnly);
  QString temp = in->getOpenFileName(this, tr("Открыть файл"), "", "*.csv");
  if (!temp.isEmpty()) {
    _d->_c = QSharedPointer<NRrls::Calc::Core>::create(_d->ui, temp);
    _d->ui->lineEdit_freq->setText(QString::number(1000));
    _d->ui->lineEdit_station1->setText(QString::number(20));
    _d->ui->lineEdit_station2->setText(QString::number(20));
    _d->ui->lineEdit_gradient->setText(QString::number(-8));
    _d->_c->setFreq(1000);
    _d->_c->setFHeight(20);
    _d->_c->setSHeight(20);
    _d->_c->setGradient(-8);
    in->hide();
    exec();
  }
}

void NRrlsMainWindow::changeHeight() {
  if (_d->ui->customplot_1->graphCount() >= 5) {
    QLineEdit *l = qobject_cast<QLineEdit *>(sender());
    double h = parse(l->text().toInt());
    (h > 0) ? l->setText(QString::number(h)) : l->setText("0");
    (l == _d->ui->lineEdit_station1) ? _d->_c->setFHeight(h)
                                     : _d->_c->setSHeight(h);
    exec();
  }
}

void NRrlsMainWindow::changeRrsSpec(const QString &text) {
  if (_d->ui->customplot_1->graphCount() >= 5) {
    QComboBox *c = qobject_cast<QComboBox *>(sender());
    QComboBox *j = (c == _d->ui->comboBox_rrsStation1)
                       ? _d->ui->comboBox_jobStation1
                       : _d->ui->comboBox_jobStation2;
    QLineEdit *ca = (c == _d->ui->comboBox_rrsStation1)
                        ? _d->ui->lineEdit_capStation1
                        : _d->ui->lineEdit_capStation2,
              *co = (c == _d->ui->comboBox_rrsStation1)
                        ? _d->ui->lineEdit_coefAntenna1
                        : _d->ui->lineEdit_coefAntenna2;
    QPalette *palette = new QPalette();

    j->clear();
    j->addItem(tr("Выбрать режим"));

    if (c->currentIndex() <= 0) {
      palette->setColor(QPalette::Base, Qt::white);
      ca->setReadOnly(false);
      co->setReadOnly(false);
    } else {
      palette->setColor(QPalette::Base, NRrlsMainWindow::palette().color(
                                            QWidget::backgroundRole()));
      ca->setText(QString::number(
          (c == _d->ui->comboBox_rrsStation1)
              ? _d->_c->setFPower(_d->_c->d.spec.stat[text][0])
              : _d->_c->setSPower(_d->_c->d.spec.stat[text][0])));
      co->setText(QString::number(
          (c == _d->ui->comboBox_rrsStation1)
              ? _d->_c->setFPower(_d->_c->d.spec.stat[text][1])
              : _d->_c->setSPower(_d->_c->d.spec.stat[text][1])));
      ca->setReadOnly(true);
      co->setReadOnly(true);
      for (const auto &it : _d->_c->d.spec.j[text].keys()) j->addItem(it);
    }

    ca->setPalette(*palette);
    co->setPalette(*palette);
    delete palette;
    exec();
  }
}

void NRrlsMainWindow::changeSens(const QString &text) {
  if (_d->ui->customplot_1->graphCount() >= 5) {
    QComboBox *c = qobject_cast<QComboBox *>(sender());
    QComboBox *r = (c == _d->ui->comboBox_jobStation1)
                       ? _d->ui->comboBox_rrsStation1
                       : _d->ui->comboBox_rrsStation2;
    QLineEdit *s = (c == _d->ui->comboBox_jobStation1)
                       ? _d->ui->lineEdit_sensStation1
                       : _d->ui->lineEdit_sensStation2;
    QPalette *palette = new QPalette();

    if (c->currentIndex() <= 0) {
      palette->setColor(QPalette::Base, Qt::white);
      s->setReadOnly(false);
    } else {
      palette->setColor(QPalette::Base, NRrlsMainWindow::palette().color(
                                            QWidget::backgroundRole()));
      s->setText(QString::number(
          _d->_c->setFSensetivity(_d->_c->d.spec.j[r->currentText()][text])));
      s->setReadOnly(true);
    }

    s->setPalette(*palette);
    delete palette;
    exec();
  }
}

void NRrlsMainWindow::onMouseMove(QMouseEvent *event) {
  if (_d->ui->customplot_1->graphCount() >= 5) {
    const double coef = _d->_c->xRange() / _d->_c->yRange();
    const double h = .02 * _d->_c->yRange();
    QCustomPlot *customplot_1 = qobject_cast<QCustomPlot *>(sender());

    double xa = customplot_1->xAxis->pixelToCoord(event->pos().x());

    if (_d->line != nullptr) delete _d->line;
    _d->line = new QCPItemLine(_d->ui->customplot_1);
    _d->line->setPen(QPen(QColor(Qt::darkMagenta)));
    _d->line->start->setCoords(_d->_c->coordX(xa) - coef * h,
                               _d->_c->coordY(xa));
    _d->line->end->setCoords(_d->_c->coordX(xa) + coef * h, _d->_c->coordY(xa));

    if (_d->l != nullptr) delete _d->l;
    _d->l = new QCPItemLine(_d->ui->customplot_1);
    _d->l->setPen(QPen(QColor(Qt::darkMagenta)));
    _d->l->start->setCoords(_d->_c->coordX(xa), _d->_c->coordY(xa) - h);
    _d->l->end->setCoords(_d->_c->coordX(xa), _d->_c->coordY(xa) + h);
    _d->ui->customplot_1->replot();
  }
}

void NRrlsMainWindow::openFirstStation() {
  if (_d->ui->customplot_1->graphCount() >= 5) {
    _f = new FirstStationWindow();
    _f->show();
  }
}

void NRrlsMainWindow::onCustomPlotClicked(QMouseEvent *event) {
  std::cout << " h";
}
