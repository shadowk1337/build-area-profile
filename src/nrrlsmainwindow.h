#ifndef NRRLSMMAINWINDOW_H
#define NRRLSMMAINWINDOW_H

#include <QHBoxLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>

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
  void setSettings(const QVariantMap &options);
  void setWidgets();
  void setDebugLevel(int level);

 private:
  struct Private;
  Private *const _d;
  QHBoxLayout *la;
  QLineEdit *l;
  QPushButton *pushButton_fileDial;

 private slots:
  void onMouseMove(QMouseEvent *event);
  void tabPressed(QEvent *event);
};

#endif  // NRRLSMMAINWINDOW_H
