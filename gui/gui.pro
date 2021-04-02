#QT += sql
QT += widgets                       \
      core                          \
      printsupport
#      gui \
#      widgets

#TEMPLATE = app
TARGET = rvision_rrls_gui
DESTDIR = ../../bin

CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH +=                      \
    ../src                          \
    ../qcustomplot

SOURCES +=                          \
    ../src/gui.cpp                  \
    ../src/nrrlsmainwindow.cpp      \
    ../src/firststationwindow.cpp   \
    ../src/nrrlslogcategory.cpp     \
    ../src/nrrlscalc.cpp            \
    ../qcustomplot/qcustomplot.cpp

HEADERS +=                          \
    ../src/nrrlsmainwindow.h        \
    ../src/firststationwindow.h   \
    ../src/nrrlslogcategory.h       \
    ../src/nrrlscalc.h              \
    ../qcustomplot/qcustomplot.h

FORMS +=                            \
    ../src/nrrlsmainwindow.ui       \
    ../src/firststationwindow.ui
