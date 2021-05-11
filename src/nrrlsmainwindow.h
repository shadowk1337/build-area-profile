#ifndef NRRLSMMAINWINDOW_H
#define NRRLSMMAINWINDOW_H

#include "nrrlscoordswindow.h"
#include "nrrlsdiagramwindow.h"
#include "nrrlsfirststationwidget.h"
#include "nrrlssecondstationwidget.h"

/**
 * Главное окно
 */
class NRrlsMainWindow : public QMainWindow {
  Q_OBJECT

 public:
  NRrlsMainWindow(const QVariantMap &options, QWidget *parent = nullptr);

  ~NRrlsMainWindow();

  void init();

  void saveSettings();
  void restoreSettings();

 private:
  void exec();
  void setSettings(const QVariantMap &options);
  void setWidgets();
  void setDebugLevel(int level);

  void setToolBar();
  void setMainWindow();
  void setFile(const QString &filename);

 private:
  struct Private;
  Private *const _d;
  NRrlsFirstStationWidget *_f;   ///< Окно РРС1
  NRrlsSecondStationWidget *_s;  ///< Окно РРС2
  NRrlsCoordsWindow *_co;        ///< Окно координат
  NRrlsDiagramWindow *_di;       ///<  Окно диграммы
  double _xa;  ///< Абсцисса перекрестия на графике

 private:
  void capacityNotNull() const;

 private:
  void dragEnterEvent(QDragEnterEvent *event);
  void dropEvent(QDropEvent *event);

 private slots:
  void onSetFile();
  void onChangeHeight(double d);
  void onChangeSens(const QString &text);
  void onChangeRrsSpec(const QString &text);
  void onMouseMove(QMouseEvent *event);
  void onApplyButtonEntered();

 private slots:
  void onCustomPlotClicked(QMouseEvent *event);
};

#endif  // NRRLSMMAINWINDOW_H
