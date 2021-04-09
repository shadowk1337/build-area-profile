#include <QFileDialog>

#include "nrrlscalc.h"
#include "nrrlslogcategory.h"
#include "nrrlsmainwindow.h"

inline int parse(int h) { return (h > 100) ? 100 : (h < 1) ? 1 : h; }

struct NRrlsMainWindow::Private {
  Private() {}

  Ui::NRrlsMainWindow *ui = nullptr;
  QSharedPointer<NRrls::Calc::Core> _c;
  QCPItemLine *h_line = nullptr, *v_line = nullptr;
};

double freq, h1, h2;

NRrlsMainWindow::NRrlsMainWindow(const QVariantMap &options, QWidget *parent)
    : QMainWindow(parent), _d(new Private) {
  _d->ui = new Ui::NRrlsMainWindow;
  _f = new FirstStationWindow();
  _s = new SecondStationWindow();
  _c = new CoordsWindow();
  _di = new DiagramWindow();

  _d->ui->setupUi(this);

  setWindowTitle(tr("РРЛС"));
  int w = 1360;
  int h = 768;
  const QRect &r = qApp->desktop()->screen()->rect();
  setGeometry(qAbs(r.width() - w) / 2, qAbs(r.height() - h) / 2, w, h);

  setSettings(options);
  setMouseTracking(true);

  _d->ui->customplot->xAxis->setVisible(0);
  _d->ui->customplot->yAxis->setVisible(0);

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
    _d->ui->customplot->replot();
  });

  connect(_d->ui->action_13, &QAction::triggered, this, [&]() {
    if (_d->ui->action_13->text() == "Скрыть нижнюю панель") {
      _d->ui->action_13->setText(tr("Показать нижнюю панель"));
      _d->ui->frame_hider->hide();
      _d->ui->customplot->resize(1341, 731);
      _d->ui->frame->resize(1341, 731);
    } else {
      _d->ui->action_13->setText(tr("Скрыть нижнюю панель"));
      _d->ui->frame_hider->show();
      _d->ui->customplot->resize(1341, 441);
      _d->ui->frame->resize(1341, 441);
    }
  });

  connect(_d->ui->action_4, &QAction::triggered, this,
          [&]() { QApplication::exit(); });

  connect(_d->ui->lineEdit_freq, &QLineEdit::editingFinished, [&]() {
    freq = _d->ui->lineEdit_freq->text().toDouble();
    if (_d->ui->customplot->graphCount() >= 5) {
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
    if (_d->ui->customplot->graphCount() >= 5)
      _d->_c->setPower(_d->_c->data->spec.p.first,
                       _d->ui->lineEdit_capStation1->text().toDouble());
  });

  connect(_d->ui->lineEdit_capStation2, &QLineEdit::editingFinished, [&]() {
    if (_d->ui->customplot->graphCount() >= 5)
      _d->_c->setPower(_d->_c->data->spec.p.second,
                       _d->ui->lineEdit_capStation2->text().toDouble());
  });

  connect(_d->ui->lineEdit_sensStation1, &QLineEdit::editingFinished, [&]() {
    if (_d->ui->customplot->graphCount() >= 5)
      _d->_c->setSensitivity(_d->_c->data->spec.s.first,
                             _d->ui->lineEdit_sensStation1->text().toDouble());
  });

  connect(_d->ui->lineEdit_sensStation2, &QLineEdit::editingFinished, [&]() {
    if (_d->ui->customplot->graphCount() >= 5)
      _d->_c->setSensitivity(_d->_c->data->spec.s.second,
                             _d->ui->lineEdit_sensStation2->text().toDouble());
  });

  connect(_d->ui->lineEdit_coefAntenna1, &QLineEdit::editingFinished, [&]() {
    if (_d->ui->customplot->graphCount() >= 5)
      _d->_c->setCoef(_d->_c->data->tower.c.first,
                      _d->ui->lineEdit_coefAntenna1->text().toDouble());
  });

  connect(_d->ui->lineEdit_coefAntenna2, &QLineEdit::editingFinished, [&]() {
    if (_d->ui->customplot->graphCount() >= 5)
      _d->_c->setCoef(_d->_c->data->tower.c.second,
                      _d->ui->lineEdit_coefAntenna2->text().toDouble());
  });

  connect(_d->ui->lineEdit_gradient, &QLineEdit::editingFinished, [&]() {
    if (_d->ui->customplot->graphCount() >= 5) {
      _d->_c->setGradient(_d->ui->lineEdit_gradient->text().toDouble());
      exec();
    }
  });

  connect(_d->ui->pushButton_diagram, &QPushButton::clicked, [&]() {
    if (_d->ui->customplot->graphCount() >= 5) {
      if (_di != nullptr) delete _di;
      _di = new DiagramWindow();

      exec();

      _di->init(_d->_c);
      _di->exec();
      _di->show();
    }
  });

  connect(_d->ui->lineEdit_temperature, &QLineEdit::editingFinished, [&]() {
    _d->_c->setTemperature(_d->ui->lineEdit_temperature->text().toDouble());
    exec();
  });

  connect(_d->ui->lineEdit_feederAntenna1, &QLineEdit::editingFinished, [&]() {
    _d->_c->setFeedAtten(_d->_c->data->tower.wf.first,
                         _d->ui->lineEdit_feederAntenna1->text().toDouble());
  });

  connect(_d->ui->lineEdit_feederAntenna2, &QLineEdit::editingFinished, [&]() {
    _d->_c->setFeedAtten(_d->_c->data->tower.wf.second,
                         _d->ui->lineEdit_feederAntenna2->text().toDouble());
  });

  connect(_d->ui->pushButton_addStation1, &QPushButton::clicked, this,
          &NRrlsMainWindow::openFirstStation);

  connect(_d->ui->pushButton_addStation2, &QPushButton::clicked, this,
          &NRrlsMainWindow::openSecondStation);

  connect(_d->ui->customplot, &QCustomPlot::mouseDoubleClick, this,
          &NRrlsMainWindow::onCustomPlotClicked);

  connect(_d->ui->customplot, &QCustomPlot::mouseMove, this,
          &NRrlsMainWindow::onMouseMove);
}

NRrlsMainWindow::~NRrlsMainWindow() {
  delete _d->ui;
  delete _d->h_line;
  delete _d->v_line;
  delete _f;
  delete _s;
  delete _c;
  delete _di;
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
  if (_d->ui->customplot->graphCount() >= 5) {
    QLineEdit *l = qobject_cast<QLineEdit *>(sender());
    double h = parse(l->text().toInt());
    (h > 1) ? l->setText(QString::number(h)) : l->setText("1");
    (l == _d->ui->lineEdit_station1) ? _d->_c->setFHeight(h)
                                     : _d->_c->setSHeight(h);
    exec();
  }
}

void NRrlsMainWindow::changeRrsSpec(const QString &text) {
  if (_d->ui->customplot->graphCount() >= 5) {
    QComboBox *c = qobject_cast<QComboBox *>(sender());
    QComboBox *j = (c == _d->ui->comboBox_rrsStation1)
                       ? _d->ui->comboBox_jobStation1
                       : _d->ui->comboBox_jobStation2;
    QLineEdit *capacity = (c == _d->ui->comboBox_rrsStation1)
                              ? _d->ui->lineEdit_capStation1
                              : _d->ui->lineEdit_capStation2,
              *coef = (c == _d->ui->comboBox_rrsStation1)
                          ? _d->ui->lineEdit_coefAntenna1
                          : _d->ui->lineEdit_coefAntenna2;
    QPalette *palette = new QPalette();

    j->clear();
    j->addItem(tr("Выбрать режим"));

    if (c->currentIndex() <= 0) {
      palette->setColor(QPalette::Base, Qt::white);
      capacity->setReadOnly(false);
      coef->setReadOnly(false);
    } else {
      palette->setColor(QPalette::Base, NRrlsMainWindow::palette().color(
                                            QWidget::backgroundRole()));
      capacity->setText(QString::number(_d->_c->setPower(
          (c == _d->ui->comboBox_rrsStation1) ? _d->_c->data->spec.p.first
                                              : _d->_c->data->spec.p.second,
          _d->_c->data->spec.stat[text][0])));
      coef->setText(QString::number(_d->_c->setCoef(
          (c == _d->ui->comboBox_rrsStation1) ? _d->_c->data->tower.c.first
                                              : _d->_c->data->tower.c.second,
          _d->_c->data->spec.stat[text][1])));
      capacity->setReadOnly(true);
      coef->setReadOnly(true);
      for (const auto &it : _d->_c->data->spec.j[text].keys()) j->addItem(it);
    }

    capacity->setPalette(*palette);
    coef->setPalette(*palette);
    delete palette;
    exec();
  }
}

// TODO: доделать
void NRrlsMainWindow::changeSens(const QString &text) {
  if (_d->ui->customplot->graphCount() >= 5) {
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
      s->setText(QString::number(_d->_c->setSensitivity(
          (c == _d->ui->comboBox_rrsStation1) ? _d->_c->data->spec.s.first
                                              : _d->_c->data->spec.s.second,
          _d->_c->data->spec.j[r->currentText()][text])));
      std::cerr << _d->_c->data->spec.s.first << ' ';
      s->setReadOnly(true);
    }

    s->setPalette(*palette);
    delete palette;
    exec();
  }
}

void NRrlsMainWindow::openFirstStation() {
  if (_d->ui->customplot->graphCount() >= 5) {
    if (_f != nullptr) delete _f;
    _f = new FirstStationWindow();
    _f->show();
  }
}

void NRrlsMainWindow::openSecondStation() {
  if (_d->ui->customplot->graphCount() >= 5) {
    if (_c != nullptr) delete _c;
    _s = new SecondStationWindow();
    _s->show();
  }
}

void NRrlsMainWindow::onMouseMove(QMouseEvent *event) {
  if (_d->ui->customplot->graphCount() >= 5) {
    const double coef = _d->_c->xRange() / _d->_c->yRange();
    const double h = .02 * _d->_c->yRange();  ///< Высота перекрестия

    QCustomPlot *customplot = qobject_cast<QCustomPlot *>(sender());

    _xa = customplot->xAxis->pixelToCoord(event->pos().x());

    if (_d->h_line != nullptr) delete _d->h_line;

    _d->h_line = new QCPItemLine(_d->ui->customplot);
    _d->h_line->setPen(QPen(QColor(Qt::darkMagenta)));
    _d->h_line->start->setCoords(_d->_c->coordX(_xa) - coef * h,
                                 _d->_c->coordY(_xa));
    _d->h_line->end->setCoords(_d->_c->coordX(_xa) + coef * h,
                               _d->_c->coordY(_xa));

    if (_d->v_line != nullptr) delete _d->v_line;

    _d->v_line = new QCPItemLine(_d->ui->customplot);
    _d->v_line->setPen(QPen(QColor(Qt::darkMagenta)));
    _d->v_line->start->setCoords(_d->_c->coordX(_xa), _d->_c->coordY(_xa) - h);
    _d->v_line->end->setCoords(_d->_c->coordX(_xa), _d->_c->coordY(_xa) + h);

    _d->ui->customplot->replot();
  }
}

void NRrlsMainWindow::onCustomPlotClicked(QMouseEvent *event) {
  if (_d->ui->customplot->graphCount() >= 5) {
    if (_c != nullptr) delete _c;

    _c = new CoordsWindow();
    _c->setGeometry(NRrlsMainWindow::x() + event->pos().x(),
                    NRrlsMainWindow::y() + event->pos().y(), _c->width(),
                    _c->height());

    _c->init(_d->_c->data->param.coords.lowerBound(_xa).key(),
             _d->_c->data->constant.area_length -
                 _d->_c->data->param.coords.lowerBound(_xa).key(),
             _d->_c->data->param.los.first *
                     _d->_c->data->param.coords.lowerBound(_xa).key() +
                 _d->_c->data->param.los.second -
                 (_d->_c->data->param.coordsAndEarth.lowerBound(_xa).value() -
                  _d->_c->data->param.coords.lowerBound(_xa).value()),
             _d->_c->data->param.coords.lowerBound(_xa).value(),
             _d->_c->data->param.coordsAndEarth.lowerBound(_xa).value() -
                 _d->_c->data->param.coords.lowerBound(_xa).value(),
             _d->_c->data->param.H.lowerBound(_xa).value(),
             _d->_c->data->param.H_null.lowerBound(_xa).value());

    _c->show();
  }
}
