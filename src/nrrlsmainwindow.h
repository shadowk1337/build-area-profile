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
  NRrlsFirstStationWidget *_f = nullptr;   ///< Окно РРС1
  NRrlsSecondStationWidget *_s = nullptr;  ///< Окно РРС2
  NRrlsCoordsWindow *_co = nullptr;        ///< Окно координат
  NRrlsDiagramWindow *_di = nullptr;       ///<  Окно диграммы
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
  void onPlottableClicked(QCPAbstractPlottable *plottable, int dataindex,
                          QMouseEvent *event);
  void onCustomPlotDoubleClicked(QMouseEvent *event);
  void onMousePressed(QMouseEvent *event);
  void onMouseReleased(QMouseEvent *event);
};

#endif  // NRRLSMMAINWINDOW_H
