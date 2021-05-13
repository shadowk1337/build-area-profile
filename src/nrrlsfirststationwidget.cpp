#include "nrrlsfirststationwidget.h"
#include "ui_nrrlsfirststationwidget.h"

NRrlsFirstStationWidget::NRrlsFirstStationWidget(
    QSharedPointer<NRrls::Calc::Core> c, QWidget *parent)
    : QWidget(parent), _c(c), ui(new Ui::NRrlsFirstStationWidget) {
  ui->setupUi(this);

  setWindowTitle(tr("РРС 1"));

  setConnects();
}

NRrlsFirstStationWidget::~NRrlsFirstStationWidget() { delete ui; }

void NRrlsFirstStationWidget::setConnects() {
  connect(ui->rrs1HeightSpinBox,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          &NRrlsFirstStationWidget::onChangeHeight);
}

void NRrlsFirstStationWidget::capacityNotNull() const {
  //  _c->ui->applyPushButton->setEnabled(
  //      (!_c->data->spec.p.first || !_d->_c->data->spec.p.second) ? 0 : 1);
}

void NRrlsFirstStationWidget::onChangeHeight(double d) {
  QDoubleSpinBox *sp = qobject_cast<QDoubleSpinBox *>(sender());
  (sp == ui->rrs1HeightSpinBox) ? _c->data->tower.f.setY(d)
                                : _c->data->tower.s.setY(d);
  capacityNotNull();
}
