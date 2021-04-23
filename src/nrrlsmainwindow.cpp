#include <QFileDialog>

#include "nrrlscalc.h"
#include "nrrlslogcategory.h"
#include "nrrlsmainwindow.h"

inline int parse(int h) { return (h > 100) ? 100 : (h < 0) ? 0 : h; }

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

  setSettings(options);
  setMouseTracking(true);

  _d->ui->customplot->xAxis->setVisible(0);
  _d->ui->customplot->yAxis->setVisible(0);
  _d->ui->frame_result->setVisible(0);
  _d->ui->pushButton_diagram->setEnabled(0);
  _d->ui->pushButton_apply->setEnabled(0);

  _d->ui->comboBox_rrsStation1->addItem(tr("Выбрать станцию"));
  _d->ui->comboBox_rrsStation2->addItem(tr("Выбрать станцию"));
  _d->ui->comboBox_rrsStation1->addItem(tr("Р-419МЦ"));
  _d->ui->comboBox_rrsStation2->addItem(tr("Р-419МЦ"));

  _d->ui->comboBox_jobStation1->addItem(tr("Выбрать режим"));
  _d->ui->comboBox_jobStation2->addItem(tr("Выбрать режим"));

  connect(_d->ui->action_fileOpen, &QAction::triggered, this,
          &NRrlsMainWindow::onSetFile);

  connect(_d->ui->action_fileReplot, &QAction::triggered, this, [&]() {
    _d->ui->customplot->replot();
    _capacityNotNull();
  });

  connect(_d->ui->action_13, &QAction::triggered, this, [&]() {
    if (_d->ui->action_13->text() == "Скрыть нижнюю панель") {
      _d->ui->action_13->setText(tr("Показать нижнюю панель"));
      _d->ui->frame_hider->hide();
    } else {
      _d->ui->action_13->setText(tr("Скрыть нижнюю панель"));
      _d->ui->frame_hider->show();
    }
    _capacityNotNull();
  });

  connect(_d->ui->action_4, &QAction::triggered, this, [&]() {
    qApp->exit();
    _capacityNotNull();
  });

  connect(_d->ui->action_10, &QAction::triggered, this, [&]() {
    if (NRrlsMainWindow::isFullScreen()) {
      _d->ui->action_10->setText(tr("Полноэкранный режим"));
      NRrlsMainWindow::showNormal();
    } else {
      _d->ui->action_10->setText(tr("Оконный режим"));
      NRrlsMainWindow::showFullScreen();
    }
    _capacityNotNull();
  });

  connect(_d->ui->action_11, &QAction::triggered, this, [&]() {
    QPixmap pixmap(_d->ui->customplot->rect().size());
    _d->ui->customplot->render(&pixmap, QPoint(),
                               QRegion(_d->ui->customplot->rect()));
    pixmap.save(QString("screenshot_%1-%2-%3.png")
                    .arg(QDate::currentDate().year())
                    .arg(QDate::currentDate().month())
                    .arg(QDate::currentDate().day()));
    _capacityNotNull();
  });

  connect(_d->ui->lineEdit_freq, &QLineEdit::textEdited, [&]() {
    freq = _d->ui->lineEdit_freq->text().toDouble();
    if (_d->ui->customplot->graphCount() >= 5) {
      _d->_c->setFreq(freq);
      _capacityNotNull();
    }
  });

  connect(_d->ui->lineEdit_station1, &QLineEdit::textEdited, this,
          &NRrlsMainWindow::onChangeHeight);

  connect(_d->ui->lineEdit_station2, &QLineEdit::textEdited, this,
          &NRrlsMainWindow::onChangeHeight);

  connect(_d->ui->comboBox_rrsStation1, &QComboBox::currentTextChanged, this,
          &NRrlsMainWindow::onChangeRrsSpec);

  connect(_d->ui->comboBox_rrsStation2, &QComboBox::currentTextChanged, this,
          &NRrlsMainWindow::onChangeRrsSpec);

  connect(_d->ui->comboBox_jobStation1, &QComboBox::currentTextChanged, this,
          &NRrlsMainWindow::onChangeSens);

  connect(_d->ui->comboBox_jobStation2, &QComboBox::currentTextChanged, this,
          &NRrlsMainWindow::onChangeSens);

  connect(_d->ui->lineEdit_capStation1, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot->graphCount() >= 5) {
      _d->_c->setValue(_d->_c->data->spec.p.first,
                       _d->ui->lineEdit_capStation1->text().toDouble());
      _capacityNotNull();
    }
  });

  connect(_d->ui->lineEdit_capStation2, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot->graphCount() >= 5) {
      _d->_c->setValue(_d->_c->data->spec.p.second,
                       _d->ui->lineEdit_capStation2->text().toDouble());
      _capacityNotNull();
    }
  });

  connect(_d->ui->lineEdit_sensStation1, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot->graphCount() >= 5) {
      _d->_c->setValue(_d->_c->data->spec.s.first,
                       _d->ui->lineEdit_sensStation1->text().toDouble());
      _capacityNotNull();
    }
  });

  connect(_d->ui->lineEdit_sensStation2, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot->graphCount() >= 5) {
      _d->_c->setValue(_d->_c->data->spec.s.second,
                       _d->ui->lineEdit_sensStation2->text().toDouble());
      _capacityNotNull();
    }
  });

  connect(_d->ui->lineEdit_coefAntenna1, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot->graphCount() >= 5) {
      _d->_c->setValue(_d->_c->data->tower.c.first,
                       _d->ui->lineEdit_coefAntenna1->text().toDouble());
      _capacityNotNull();
    }
  });

  connect(_d->ui->lineEdit_coefAntenna2, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot->graphCount() >= 5) {
      _d->_c->setValue(_d->_c->data->tower.c.second,
                       _d->ui->lineEdit_coefAntenna2->text().toDouble());
      _capacityNotNull();
    }
  });

  connect(_d->ui->lineEdit_gradient, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot->graphCount() >= 5) {
      _d->_c->setValue(_d->_c->data->constant.g_standard,
                       _d->ui->lineEdit_gradient->text().toDouble() * 1e-08);
      _capacityNotNull();
    }
  });

  connect(_d->ui->pushButton_diagram, &QPushButton::clicked, [&]() {
    if (_d->ui->customplot->graphCount() >= 5) {
      _di->show();
    }
  });

  connect(_d->ui->lineEdit_temperature, &QLineEdit::textEdited, [&]() {
    _d->_c->setValue(_d->_c->data->constant.temperature,
                     _d->ui->lineEdit_temperature->text().toDouble());
    _capacityNotNull();
  });

  connect(_d->ui->lineEdit_probability, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot->graphCount() >= 5) {
      _d->_c->setValue(_d->_c->data->spec.prob,
                       _d->ui->lineEdit_probability->text().toDouble());
      _capacityNotNull();
    }
  });

  connect(_d->ui->lineEdit_feederAntenna1, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot->graphCount() >= 5) {
      _d->_c->setValue(_d->_c->data->tower.wf.first,
                       _d->ui->lineEdit_feederAntenna1->text().toDouble());
      _capacityNotNull();
    }
  });

  connect(_d->ui->lineEdit_feederAntenna2, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot->graphCount() >= 5) {
      _d->_c->setValue(_d->_c->data->tower.wf.second,
                       _d->ui->lineEdit_feederAntenna2->text().toDouble());
      _capacityNotNull();
    }
  });

  connect(_d->ui->pushButton_addStation1, &QPushButton::clicked, this,
          &NRrlsMainWindow::onOpenFirstStation);

  connect(_d->ui->pushButton_addStation2, &QPushButton::clicked, this,
          &NRrlsMainWindow::onOpenSecondStation);

  connect(_d->ui->customplot, &QCustomPlot::mouseDoubleClick, this,
          &NRrlsMainWindow::onCustomPlotClicked);

  //  connect(_d->ui->customplot, SIGNAL(QCustomPlot::plottableClick()),
  //          SLOT(NRrlsMainWindow::onPlottableClicked()),
  //          SIGNAL(QCustomPlot::mousePress()), Qt::QueuedConnection);

  connect(_d->ui->customplot, &QCustomPlot::mouseMove, this,
          &NRrlsMainWindow::onMouseMove);

  connect(_d->ui->pushButton_apply, SIGNAL(MouseEnterSignal()), this,
          SLOT(NRrlsMainWindow::onApplyButtonEntered()));

  connect(_d->ui->pushButton_apply, &QPushButton::clicked, this, [&]() {
    if (_d->ui->customplot->graphCount() >= 5) {
      exec();

      // Обновление окна диаграммы
      if (_di != nullptr) delete _di;
      _di = new DiagramWindow();

      _di->init(_d->_c);
      _di->exec();

      _d->ui->pushButton_apply->setEnabled(0);
      _d->ui->pushButton_diagram->setEnabled(1);
      _d->ui->frame_result->setVisible(1);
    }

    _d->ui->pushButton_apply->installEventFilter(this);
  });
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

void NRrlsMainWindow::exec() {
  try {
    if (!_d->_c->exec()) throw("");
  } catch (...) {
    int ret = QMessageBox::critical(
        this, tr("Ошибка"), tr("Произошла ошибка в расчетах"), QMessageBox::Ok);
    if (ret == QMessageBox::Ok) qApp->exit();
  }
}

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

void NRrlsMainWindow::_capacityNotNull() const {
  _d->ui->pushButton_apply->setEnabled(
      (!_d->_c->data->spec.p.first || !_d->_c->data->spec.p.second) ? 0 : 1);
}

void NRrlsMainWindow::onSetFile(bool checked) {
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
    _d->_c->setValue(_d->_c->data->tower.f.second, 20);
    _d->_c->setValue(_d->_c->data->tower.s.second, 20);
    _d->_c->setValue(_d->_c->data->constant.g_standard, -8 * 1e-8);
    in->hide();

    exec();
  }
}

void NRrlsMainWindow::onChangeHeight(const QString &text) {
  if (_d->ui->customplot->graphCount() >= 5) {
    QLineEdit *l = qobject_cast<QLineEdit *>(sender());
    double h = parse(text.toInt());
    (h > 0) ? l->setText(QString::number(h)) : l->setText("0");
    (l == _d->ui->lineEdit_station1)
        ? _d->_c->setValue(_d->_c->data->tower.f.second, h)
        : _d->_c->setValue(_d->_c->data->tower.s.second, h);
    _capacityNotNull();
  }
}

void NRrlsMainWindow::onChangeRrsSpec(const QString &text) {
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
      capacity->setText(QString::number(_d->_c->setValueWithReturn(
          (c == _d->ui->comboBox_rrsStation1) ? _d->_c->data->spec.p.first
                                              : _d->_c->data->spec.p.second,
          _d->_c->data->spec.stat[text][0])));
      coef->setText(QString::number(_d->_c->setValueWithReturn(
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

    _capacityNotNull();
  }
}

// TODO: доделать
void NRrlsMainWindow::onChangeSens(const QString &text) {
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
      s->setText(QString::number(_d->_c->setValueWithReturn(
          (c == _d->ui->comboBox_jobStation1) ? _d->_c->data->spec.s.first
                                              : _d->_c->data->spec.s.second,
          _d->_c->data->spec.j[r->currentText()][text])));
      s->setReadOnly(true);
    }

    s->setPalette(*palette);
    delete palette;

    _capacityNotNull();
  }
}

void NRrlsMainWindow::onOpenFirstStation() {
  if (_d->ui->customplot->graphCount() >= 5) {
    if (_f != nullptr) delete _f;
    _f = new FirstStationWindow();
    _f->show();
  }
}

void NRrlsMainWindow::onOpenSecondStation() {
  if (_d->ui->customplot->graphCount() >= 5) {
    if (_c != nullptr) delete _c;
    _s = new SecondStationWindow();
    _s->show();
  }
}

void NRrlsMainWindow::onMouseMove(QMouseEvent *event) {
  if (_d->ui->customplot->graphCount() >= 5) {
    const double h = .005 * _d->_c->xRange();  ///< Длина перекрестия
    const double v = h * (_d->_c->yRange() / _d->_c->xRange()) *
                     _d->ui->customplot->width() / _d->ui->customplot->height();

    QCustomPlot *customplot = qobject_cast<QCustomPlot *>(sender());

    _xa = customplot->xAxis->pixelToCoord(event->pos().x());

    if (_d->h_line != nullptr) delete _d->h_line;

    _d->h_line = new QCPItemLine(_d->ui->customplot);
    _d->h_line->setPen(QPen(QColor(Qt::darkMagenta)));
    _d->h_line->start->setCoords(_d->_c->coordX(_xa) - h, _d->_c->coordY(_xa));
    _d->h_line->end->setCoords(_d->_c->coordX(_xa) + h, _d->_c->coordY(_xa));

    if (_d->v_line != nullptr) delete _d->v_line;

    _d->v_line = new QCPItemLine(_d->ui->customplot);
    _d->v_line->setPen(QPen(QColor(Qt::darkMagenta)));
    _d->v_line->start->setCoords(_d->_c->coordX(_xa), _d->_c->coordY(_xa) - v);
    _d->v_line->end->setCoords(_d->_c->coordX(_xa), _d->_c->coordY(_xa) + v);

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

void NRrlsMainWindow::onPlottableClicked(QCPAbstractPlottable *plottable,
                                         int dataIndex, QMouseEvent *event) {
  //  if (plottable == fr_up_idx)
  _d->_c->data->mainWindow->customplot->replot();
}

void NRrlsMainWindow::onApplyButtonEntered() {
  if (!_d->ui->pushButton_apply->isEnabled()) {
    QWhatsThis::enterWhatsThisMode();
    QWhatsThis::showText(_d->ui->pushButton_apply->pos(),
                         tr("Для вывода результатов настройте станции"));
    //    _d->ui->pushButton_apply->setWhatsThis(
    //        tr("Для вывода результатов настройте станции"));
    _d->ui->frame_result->setVisible(0);
  }
}

bool NRrlsMainWindow::eventFilter(QObject *obj, QEvent *event) {
  if (obj != _d->ui->pushButton_apply) return false;

  bool result = QObject::eventFilter(obj, event);
  switch (event->type()) {
    case QEvent::Enter:
      emit MouseEnterSignal();
      break;
    default:
      break;
  }
  return result;
}
