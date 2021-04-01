#ifndef NRRLSMMAINWINDOW_H
#define NRRLSMMAINWINDOW_H

#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>

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
  QHBoxLayout *la;
  QPushButton *fileDial, *changeWidget;

 private slots:
  void setFile(bool checked);
  void changeHeight();
  void changeRrsSpec(const QString &Stext);
  void changeSens(const QString &text);
  void onMouseMove(QMouseEvent *event);
};

#endif  // NRRLSMMAINWINDOW_H
