QT       += \
    printsupport \
    widgets \
    core

greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

CONFIG += c++1z

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../data/formules/calcformules.cpp \
    ../intervals/types/closed.cpp \
    ../intervals/types/halfopened.cpp \
    ../intervals/types/opened.cpp \
    ../intervals/intervals.cpp \
    main.cpp \
    mainwindow.cpp \
    ../Qcustomplot/qcustomplot.cpp

HEADERS += \
    ../data/formules/calcformules.h \
    ../data/datastruct.h \
    ../data/constants.h \
    ../intervals/intervals.h \
    ../Qcustomplot/qcustomplot.h \
    headings.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += \
    ../Qcustomplot \
    ../data \
    ../data/formules \
    ../intervals
    ../intervals/types \


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
