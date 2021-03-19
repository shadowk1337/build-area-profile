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

  setWindowTitle(tr("РРЛС"));
  int w = 1360;
  int h = 768;
  const QRect &r = qApp->desktop()->screen()->rect();
  setGeometry(qAbs(r.width() - w) / 2, qAbs(r.height() - h) / 2, w, h);

  changeWidget = new QPushButton(this);
  changeWidget->setGeometry(1128, 21, 221, 20);
  changeWidget->setText(tr("Высотный профиль"));
  changeWidget->hide();

  int h1, h2;  ///< Высоты, задаваемые ползунками
  double freq;

  _d->ui->comboBox_rrsStation1->addItem(tr("Выбрать станцию"));
  _d->ui->comboBox_rrsStation2->addItem(tr("Выбрать станцию"));
  _d->ui->comboBox_rrsStation1->addItem(tr("Р-419МЦ"));
  _d->ui->comboBox_rrsStation2->addItem(tr("Р-419МЦ"));

  connect(_d->ui->action_fileOpen, &QAction::triggered, [&]() {
    h1 = h2 = 20;
    freq = 1000;

    QFileDialog *in = new QFileDialog(this);
    in->setOption(QFileDialog::DontUseNativeDialog, QFileDialog::ReadOnly);
    QString temp = in->getOpenFileName(this, tr("Открыть файл"), "", "*.csv");
    changeWidget->show();
    if (!temp.isEmpty()) {
      _d->ui->customplot_1->xAxis->setVisible(1);
      _d->ui->customplot_1->yAxis->setVisible(1);
      _d->ui->label_coords->show();
      _d->ui->lineEdit_station1->setText(QString::number(h1));
      _d->ui->lineEdit_station2->setText(QString::number(h2));
      _d->ui->lineEdit_freq->setText(QString::number(freq));
      _d->_c = QSharedPointer<NRrls::Calc::Core>::create(_d->ui, temp);
      _d->_c->setFreq(_d->ui->lineEdit_freq->text().toDouble());
      _d->_c->setSenHeight(h1);
      _d->_c->setRecHeight(h2);
      in->hide();
      exec();
    }
  });

  connect(_d->ui->action_fileReplot, &QAction::triggered, [&]() {
    //    if (!fileName.isEmpty()) {
    exec();
    _d->ui->customplot_1->replot();
    _d->ui->customplot_2->replot();
    //    }
  });

  connect(_d->ui->action_13, &QAction::triggered, [&]() {
    if (_d->ui->action_13->text() == "Скрыть нижнюю панель") {
      _d->ui->action_13->setText(tr("Показать нижнюю панель"));
      _d->ui->widget->hide();
      _d->ui->customplot_1->resize(1341, 731);
      _d->ui->frame->resize(1341, 731);
    } else {
        _d->ui->action_13->setText(tr("Скрыть нижнюю панель"));
        _d->ui->widget->show();
        _d->ui->customplot_1->resize(1341, 521);
        _d->ui->frame->resize(1341, 521);
      }
  });

  connect(_d->ui->lineEdit_freq, &QLineEdit::textEdited, [&]() {
    freq = _d->ui->lineEdit_freq->text().toDouble();
    if (_d->ui->customplot_1->graphCount() >= 5) {
      _d->_c->setFreq(freq);
      _d->ui->comboBox_feeder->clear();
      if (freq >= .1 && freq <= 10) {
        _d->ui->comboBox_feeder->addItems(
            {tr("РК-50-4-13"), tr("РК-50-7-11"), tr("РК-50-7-13")});
        _d->ui->comboBox_feeder->addItems(
            {tr("РК-50-7-25"), tr("РК-50-7-27"), tr("РК-50-7-32")});
      }
      if (freq >= .1 && freq <= 3)
        _d->ui->comboBox_feeder->addItems(
            {tr("РК-50-11-11"), tr("РК-50-13-15"), tr("РК-50-11-21")});
      if (freq == 3) _d->ui->comboBox_feeder->addItem(tr("МЭК-22"));
      if (freq == 6) _d->ui->comboBox_feeder->addItem(tr("МЭК-58"));
      if (freq == 10) _d->ui->comboBox_feeder->addItem(tr("МЭК-84"));
      if (freq == 15) _d->ui->comboBox_feeder->addItem(tr("МЭК-120"));
      if (freq == 20) _d->ui->comboBox_feeder->addItem(tr("МЭК-180"));
      if (freq == 30) _d->ui->comboBox_feeder->addItem(tr("МЭК-26"));
      exec();
    }
  });

  connect(_d->ui->lineEdit_station1, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5) {
      h1 = heightParse(_d->ui->lineEdit_station1->text().toInt());
      if (h1) _d->ui->lineEdit_station1->setText(QString::number(h1));
      _d->_c->setSenHeight(h1);
      exec();
    }
  });

  //  connect(_d->ui->comboBox_rrsStation1, &QComboBox::currentTextChanged,
  //  this,
  //          &NRrlsMainWindow::onComboBoxValueChanged);

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

  connect(_d->ui->lineEdit_station2, &QLineEdit::textEdited, [&]() {
    if (_d->ui->customplot_1->graphCount() >= 5) {
      h2 = heightParse(_d->ui->lineEdit_station2->text().toInt());
      if (h2) _d->ui->lineEdit_station2->setText(QString::number(h2));
      _d->_c->setRecHeight(h2);
      exec();
    }
  });

  //  connect(_d->ui->lineEdit_station1Length, &QLineEdit::textEdited, [&]() {
  //    _d->_c->setSenFLength(_d->ui->lineEdit_station1Length->text().toDouble());
  //    init();
  //  });

  connect(_d->ui->customplot_1, &QCustomPlot::mouseMove, this,
          &NRrlsMainWindow::onMouseMove);
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
