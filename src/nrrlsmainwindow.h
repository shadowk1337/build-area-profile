#ifndef NRRLSMMAINWINDOW_H
#define NRRLSMMAINWINDOW_H

#include <QtCore>
#include <QtWidgets>

#include "coordswindow.h"
#include "diagramwindow.h"
#include "firststationwindow.h"
#include "secondstationwindow.h"

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

 private:
  struct Private;
  Private *const _d;
  FirstStationWindow *_f;   ///< Окно РРС1
  SecondStationWindow *_s;  ///< Окно РРС2
  CoordsWindow *_c;         ///< Окно координат
  DiagramWindow *_di;       ///<  Окно диграммы
  double _xa;  ///< Абсцисса перекрестия на графике

 private slots:
  void setFile(bool checked);
  void changeHeight();
  void changeRrsSpec(const QString &Stext);
  void changeSens(const QString &text);
  void onMouseMove(QMouseEvent *event);

 private slots:
  void onCustomPlotClicked(QMouseEvent *event);

 private slots:
  void openFirstStation();
  void openSecondStation();
};

#endif  // NRRLSMMAINWINDOW_H
