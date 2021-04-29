#include <QBitmap>
#include <QFileDialog>

#include "nrrlscalc.h"
#include "nrrlslogcategory.h"
#include "nrrlsmainwindow.h"

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
  setAcceptDrops(true);

  setMainWindow();
  setToolBar();

  connect(_d->ui->fileOpenAction, &QAction::triggered, this,
          &NRrlsMainWindow::onSetFile);

  connect(_d->ui->fileReplotAction, &QAction::triggered, this, [&]() {
    _d->ui->customplot->replot();
    capacityNotNull();
  });

  connect(_d->ui->action_13, &QAction::triggered, this, [&]() {
    if (_d->ui->action_13->text() == "Скрыть нижнюю панель") {
      _d->ui->action_13->setText(tr("Показать нижнюю панель"));
      _d->ui->hiderFrame->hide();
    } else {
      _d->ui->action_13->setText(tr("Скрыть нижнюю панель"));
      _d->ui->hiderFrame->show();
    }
    capacityNotNull();
  });

  connect(_d->ui->action_4, &QAction::triggered, this, [&]() {
    qApp->exit();
    capacityNotNull();
  });

  connect(_d->ui->action_10, &QAction::triggered, this, [&]() {
    if (NRrlsMainWindow::isFullScreen()) {
      _d->ui->action_10->setText(tr("Полноэкранный режим"));
      NRrlsMainWindow::showNormal();
    } else {
      _d->ui->action_10->setText(tr("Оконный режим"));
      NRrlsMainWindow::showFullScreen();
    }
    capacityNotNull();
  });

  connect(_d->ui->action_11, &QAction::triggered, this, [&]() {
    QPixmap pixmap(_d->ui->customplot->rect().size());
    _d->ui->customplot->render(&pixmap, QPoint(),
                               QRegion(_d->ui->customplot->rect()));
    pixmap.save(QString("screenshot_%1-%2-%3.png")
                    .arg(QDate::currentDate().year())
                    .arg(QDate::currentDate().month())
                    .arg(QDate::currentDate().day()));
    capacityNotNull();
  });

  connect(_d->ui->trackFrequencySpinBox,
          QOverload<double>::of(
              QOverload<double>::of(&QDoubleSpinBox::valueChanged)),
          [&]() {
            freq = _d->ui->trackFrequencySpinBox->value();
            if (_d->ui->customplot->graphCount() >= 5) {
              _d->_c->setFreq(freq);
              capacityNotNull();
            }
          });

  connect(_d->ui->rrs1HeightSpinBox,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          &NRrlsMainWindow::onChangeHeight);

  connect(_d->ui->rrs2HeightSpinBox,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          &NRrlsMainWindow::onChangeHeight);

  connect(_d->ui->rrs1TypeComboBox, &QComboBox::currentTextChanged, this,
          &NRrlsMainWindow::onChangeRrsSpec);

  connect(_d->ui->rrs2TypeComboBox, &QComboBox::currentTextChanged, this,
          &NRrlsMainWindow::onChangeRrsSpec);

  connect(_d->ui->rrs1ModeSpinBox, &QComboBox::currentTextChanged, this,
          &NRrlsMainWindow::onChangeSens);

  connect(_d->ui->rrs2ModeSpinBox, &QComboBox::currentTextChanged, this,
          &NRrlsMainWindow::onChangeSens);

  connect(_d->ui->rrs1CapacitySpinBox,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&]() {
            if (_d->ui->customplot->graphCount() >= 5) {
              _d->_c->setValue(_d->_c->data->spec.p.first,
                               _d->ui->rrs1CapacitySpinBox->value());
              capacityNotNull();
            }
          });

  connect(_d->ui->rrs2CapacitySpinBox,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&]() {
            if (_d->ui->customplot->graphCount() >= 5) {
              _d->_c->setValue(_d->_c->data->spec.p.second,
                               _d->ui->rrs2CapacitySpinBox->value());
              capacityNotNull();
            }
          });

  connect(_d->ui->rrs1SensitivitySpinBox,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&]() {
            if (_d->ui->customplot->graphCount() >= 5) {
              _d->_c->setValue(_d->_c->data->spec.s.first,
                               _d->ui->rrs1SensitivitySpinBox->value());
              capacityNotNull();
            }
          });

  connect(_d->ui->rrs2SensitivitySpinBox,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&]() {
            if (_d->ui->customplot->graphCount() >= 5) {
              _d->_c->setValue(_d->_c->data->spec.s.second,
                               _d->ui->rrs2SensitivitySpinBox->value());
              capacityNotNull();
            }
          });

  connect(_d->ui->rrs1CoefSpinBox,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&]() {
            if (_d->ui->customplot->graphCount() >= 5) {
              _d->_c->setValue(_d->_c->data->tower.c.first,
                               _d->ui->rrs1CoefSpinBox->value());
              capacityNotNull();
            }
          });

  connect(_d->ui->rrs2CoefSpinBox,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&]() {
            if (_d->ui->customplot->graphCount() >= 5) {
              _d->_c->setValue(_d->_c->data->tower.c.second,
                               _d->ui->rrs2CoefSpinBox->value());
              capacityNotNull();
            }
          });

  connect(_d->ui->trackGradientSpinBox,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&]() {
            if (_d->ui->customplot->graphCount() >= 5) {
              _d->_c->setValue(_d->_c->data->constant.g_standard,
                               _d->ui->trackGradientSpinBox->value() * 1e-08);
              capacityNotNull();
            }
          });

  connect(_d->ui->trackDiagramPushButton, &QPushButton::clicked, [&]() {
    if (_d->ui->customplot->graphCount() >= 5) {
      _di->show();
    }
  });

  connect(_d->ui->trackTemperatureSpinBox,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&]() {
            if (_d->ui->customplot->graphCount() >= 5) {
              _d->_c->setValue(_d->_c->data->constant.temperature,
                               _d->ui->trackTemperatureSpinBox->value());
              capacityNotNull();
            }
          });

  connect(_d->ui->trackProbabilitySpinBox,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&]() {
            if (_d->ui->customplot->graphCount() >= 5) {
              _d->_c->setValue(_d->_c->data->spec.prob,
                               _d->ui->trackProbabilitySpinBox->value());
              capacityNotNull();
            }
          });

  connect(_d->ui->rrs1FeederSpinBox,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&]() {
            if (_d->ui->customplot->graphCount() >= 5) {
              _d->_c->setValue(_d->_c->data->tower.wf.first,
                               _d->ui->rrs1FeederSpinBox->value());
              capacityNotNull();
            }
          });

  connect(_d->ui->rrs2FeederSpinBox,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&]() {
            if (_d->ui->customplot->graphCount() >= 5) {
              _d->_c->setValue(_d->_c->data->tower.wf.second,
                               _d->ui->rrs2FeederSpinBox->value());
              capacityNotNull();
            }
          });

  connect(_d->ui->customplot, &QCustomPlot::mouseDoubleClick, this,
          &NRrlsMainWindow::onCustomPlotClicked);

  connect(_d->ui->customplot, &QCustomPlot::mouseMove, this,
          &NRrlsMainWindow::onMouseMove);

  connect(_d->ui->applyPushButton, SIGNAL(MouseEnterSignal()), this,
          SLOT(NRrlsMainWindow::onApplyButtonEntered()));

  connect(_d->ui->applyPushButton, &QPushButton::clicked, this, [&]() {
    if (_d->ui->customplot->graphCount() >= 5) {
      exec();

      // Обновление окна диаграммы
      if (_di != nullptr) delete _di;
      _di = new DiagramWindow();

      _di->init(_d->_c);
      _di->exec();

      _d->ui->applyPushButton->setEnabled(0);
      _d->ui->trackDiagramPushButton->setEnabled(1);
      _d->ui->resultFrame->setVisible(1);
    }

    _d->ui->applyPushButton->installEventFilter(this);
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

void NRrlsMainWindow::capacityNotNull() const {
  _d->ui->applyPushButton->setEnabled(
      (!_d->_c->data->spec.p.first || !_d->_c->data->spec.p.second) ? 0 : 1);
}

void NRrlsMainWindow::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->hasFormat("text/csv")) event->accept();
  //  else
  //    event->ignore();
}

void NRrlsMainWindow::dropEvent(QDropEvent *event) {
  setFile(event->mimeData()->text().mid(7));
  event->accept();
}

void NRrlsMainWindow::setMainWindow() {
  QRect rect;
  rect.setWidth(1362);
  rect.setHeight(301);
  _d->ui->hiderFrame->setFrameRect(rect);

  _d->ui->customplot->xAxis->setVisible(0);
  _d->ui->customplot->yAxis->setVisible(0);
  _d->ui->resultFrame->setVisible(0);
  _d->ui->trackDiagramPushButton->setEnabled(0);
  _d->ui->applyPushButton->setEnabled(0);

  _d->ui->rrs1TypeComboBox->addItem(tr("Выбрать станцию"));
  _d->ui->rrs2TypeComboBox->addItem(tr("Выбрать станцию"));
  _d->ui->rrs1TypeComboBox->addItem(tr("Р-419МЦ"));
  _d->ui->rrs2TypeComboBox->addItem(tr("Р-419МЦ"));

  _d->ui->rrs1ModeSpinBox->addItem(tr("Выбрать режим"));
  _d->ui->rrs2ModeSpinBox->addItem(tr("Выбрать режим"));
}

void NRrlsMainWindow::setToolBar() {
//  _d->ui->rrs1Action->setIcon(QIcon(":/images/rrs1.svg"));
//  _d->ui->rrs2Action->setIcon(QIcon(":/images/rrs2.svg"));
  _d->ui->rrs1Action->setText("fdas");
}

void NRrlsMainWindow::setFile(const QString &filename) {
  _d->ui->mainStack->setCurrentIndex(1);
  _d->_c = QSharedPointer<NRrls::Calc::Core>::create(_d->ui, filename);

  _d->ui->trackFrequencySpinBox->setValue(1000);
  _d->ui->rrs1HeightSpinBox->setValue(20);
  _d->ui->rrs2HeightSpinBox->setValue(20);
  _d->ui->trackGradientSpinBox->setValue(-8);
  _d->_c->setFreq(1000);
  _d->_c->data->tower.f.setY(20);
  _d->_c->data->tower.s.setY(20);
  _d->_c->setValue(_d->_c->data->constant.g_standard, -8 * 1e-8);

  exec();
}

void NRrlsMainWindow::onSetFile() {
  QFileDialog *in = new QFileDialog(this);
  in->setOption(QFileDialog::DontUseNativeDialog, QFileDialog::ReadOnly);
  QString temp = in->getOpenFileName(this, tr("Открыть файл"), "", "*.csv");
  std::cerr << temp.toStdString() << ' ';
  if (!temp.isEmpty()) setFile(temp);
  in->hide();
}

void NRrlsMainWindow::onChangeHeight(double d) {
  if (_d->ui->customplot->graphCount() >= 5) {
    QDoubleSpinBox *sp = qobject_cast<QDoubleSpinBox *>(sender());
    (sp == _d->ui->rrs1HeightSpinBox) ? _d->_c->data->tower.f.setY(d)
                                      : _d->_c->data->tower.s.setY(d);
    capacityNotNull();
  }
}

void NRrlsMainWindow::onChangeRrsSpec(const QString &text) {
  if (_d->ui->customplot->graphCount() >= 5) {
    QComboBox *c = qobject_cast<QComboBox *>(sender());
    QComboBox *j = (c == _d->ui->rrs1TypeComboBox) ? _d->ui->rrs1ModeSpinBox
                                                   : _d->ui->rrs2ModeSpinBox;
    QDoubleSpinBox *capacity = (c == _d->ui->rrs1TypeComboBox)
                                   ? _d->ui->rrs1CapacitySpinBox
                                   : _d->ui->rrs2CapacitySpinBox,
                   *coef = (c == _d->ui->rrs1TypeComboBox)
                               ? _d->ui->rrs1CoefSpinBox
                               : _d->ui->rrs2CoefSpinBox;
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
      capacity->setValue(_d->_c->setValueWithReturn(
          (c == _d->ui->rrs1TypeComboBox) ? _d->_c->data->spec.p.first
                                          : _d->_c->data->spec.p.second,
          _d->_c->data->spec.stat[text][0]));
      coef->setValue(_d->_c->setValueWithReturn(
          (c == _d->ui->rrs1TypeComboBox) ? _d->_c->data->tower.c.first
                                          : _d->_c->data->tower.c.second,
          _d->_c->data->spec.stat[text][1]));
      capacity->setReadOnly(true);
      coef->setReadOnly(true);
      for (const auto &it : _d->_c->data->spec.j[text].keys()) j->addItem(it);
    }

    capacity->setPalette(*palette);
    coef->setPalette(*palette);
    delete palette;

    capacityNotNull();
  }
}

void NRrlsMainWindow::onChangeSens(const QString &text) {
  if (_d->ui->customplot->graphCount() >= 5) {
    QComboBox *c = qobject_cast<QComboBox *>(sender());
    QComboBox *r = (c == _d->ui->rrs1ModeSpinBox) ? _d->ui->rrs1TypeComboBox
                                                  : _d->ui->rrs2TypeComboBox;
    QDoubleSpinBox *s = (c == _d->ui->rrs1ModeSpinBox)
                            ? _d->ui->rrs1SensitivitySpinBox
                            : _d->ui->rrs2SensitivitySpinBox;
    QPalette *palette = new QPalette();

    if (c->currentIndex() <= 0) {
      palette->setColor(QPalette::Base, Qt::white);
      s->setReadOnly(false);
    } else {
      palette->setColor(QPalette::Base, NRrlsMainWindow::palette().color(
                                            QWidget::backgroundRole()));
      s->setValue(_d->_c->setValueWithReturn(
          (c == _d->ui->rrs1ModeSpinBox) ? _d->_c->data->spec.s.first
                                         : _d->_c->data->spec.s.second,
          _d->_c->data->spec.j[r->currentText()][text]));
      s->setReadOnly(true);
    }

    s->setPalette(*palette);
    delete palette;

    capacityNotNull();
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

void NRrlsMainWindow::onApplyButtonEntered() {
  if (!_d->ui->applyPushButton->isEnabled()) {
    QWhatsThis::enterWhatsThisMode();
    QWhatsThis::showText(_d->ui->applyPushButton->pos(),
                         tr("Для вывода результатов настройте станции"));
    _d->ui->resultFrame->setVisible(0);
  }
}
