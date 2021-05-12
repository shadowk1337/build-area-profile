QT += widgets                       \
      core                          \
      gui

#TEMPLATE = app
TARGET = rvision_rrls_gui
DESTDIR = ../../bin

CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH +=                          \
    ../src                              \
    ../qcustomplot

SOURCES +=                              \
    ../src/nrrlsgraphpainter.cpp        \
    ../src/nrrlsgui.cpp                 \
    ../src/nrrlsmainwindow.cpp          \
    ../src/nrrlscoordswindow.cpp        \
    ../src/nrrlsdiagramwindow.cpp       \
    ../src/nrrlslogcategory.cpp         \
    ../src/nrrlscalc.cpp                \
    ../qcustomplot/qcustomplot.cpp      \
    ../src/nrrlsfirststationwidget.cpp  \
    ../src/nrrlssecondstationwidget.cpp

HEADERS +=                              \
    ../src/nrrlsgraphpainter.h          \
    ../src/nrrlsmainwindow.h            \
    ../src/nrrlscoordswindow.h          \
    ../src/nrrlsdiagramwindow.h         \
    ../src/nrrlslogcategory.h           \
    ../src/nrrlscalc.h                  \
    ../qcustomplot/qcustomplot.h        \
    ../src/nrrlsfirststationwidget.h    \
    ../src/nrrlssecondstationwidget.h

FORMS +=                                \
    ../src/nrrlsmainwindow.ui           \
    ../src/nrrlscoordswindow.ui         \
    ../src/nrrlsdiagramwindow.ui        \
    ../src/nrrlsfirststationwidget.ui   \
    ../src/nrrlssecondstationwidget.ui

RESOURCES += \
    ../src/nrrlsimages.qrc
