#include <QApplication>
#include <QtCore>

#include "nrrlsmainwindow.h"

namespace NRrls {

class Options {
 public:
  Options() { _data["config"] = qApp->applicationDirPath() + "/conf/rrls.ini"; }

  QVariantMap data() { return _data; }

  int error() const { return _error; }

  bool init(const QStringList &args) {
    QStringListIterator it(args);
    while (it.hasNext()) {
      QString t = it.next();
      if (t == "--help" || t == "-h") {
        showHelp();
        return false;
      }
      if (read(t, {"config", "c"}, it)) continue;
      if (read(t, {"level", "L"}, it)) continue;
    }
    return true;
  }

  void showHelp() {
    const char *tmp =
        "Программа технологического управления %1.\n"
        "Использование программы:\n"
        "  %2 [КЛЮЧ] [ЗНАЧЕНИЕ]\n"
        "где:\n"
        "  -h, --help                   выводит справочную информацию\n\n";

    QTextStream stream(stderr);
    stream << QString(tmp).arg("РРЛС").arg(qAppName());
  }

 private:
  bool read(const QString &value, const QStringList &name,
            QStringListIterator &it) {
    const QString &l = name.at(0);
    const QString &s = name.at(1);
    if (value == "--" + l || value == "-" + s) {
      if (it.hasNext()) {
        _data[l] = it.next();
      }
      return true;
    }
    return false;
  }

 private:
  int _error = 0;
  QVariantMap _data;
};

}  // namespace NRrls

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  QApplication::setOrganizationName("Niissu");
  QApplication::setOrganizationDomain("niissu.ru");

  NRrls::Options options;
  if (!options.init(app.arguments())) {
    return options.error();
  } else {
    auto data = options.data();
  }

  NRrlsMainWindow w(options.data());
  QPalette p;
  p.setBrush(QPalette::Background, QBrush(QColor(Qt::white)));
  w.setPalette(p);
  w.restoreSettings();
//  w.init();

  w.show();
  int res = app.exec();
  w.saveSettings();
  return res;

  return app.exec();
}
