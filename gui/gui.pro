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
    ../src/nrrlsgraphpainter.cpp         \
    ../src/nrrlsgui.cpp                  \
    ../src/nrrlsmainwindow.cpp      \
    ../src/nrrlsfirststationwindow.cpp   \
    ../src/nrrlssecondstationwindow.cpp  \
    ../src/nrrlscoordswindow.cpp         \
    ../src/nrrlsdiagramwindow.cpp        \
    ../src/nrrlslogcategory.cpp     \
    ../src/nrrlscalc.cpp            \
    ../qcustomplot/qcustomplot.cpp

HEADERS +=                          \
    ../src/nrrlsgraphpainter.h           \
    ../src/nrrlsmainwindow.h        \
    ../src/nrrlsfirststationwindow.h     \
    ../src/nrrlssecondstationwindow.h    \
    ../src/nrrlscoordswindow.h           \
    ../src/nrrlsdiagramwindow.h          \
    ../src/nrrlslogcategory.h       \
    ../src/nrrlscalc.h              \
    ../qcustomplot/qcustomplot.h

FORMS +=                            \
    ../src/nrrlsmainwindow.ui       \
    ../src/nrrlsfirststationwindow.ui    \
    ../src/nrrlssecondstationwindow.ui   \
    ../src/nrrlscoordswindow.ui          \
    ../src/nrrlsdiagramwindow.ui

RESOURCES += \
    ../src/nrrlsimages.qrc
