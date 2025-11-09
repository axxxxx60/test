QT += core gui network sql charts
QT += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# libiio头文件路径
INCLUDEPATH += $$PWD/libiio/include

# libiio库文件路径
LIBS += -L$$PWD/libiio/Windows-MinGW-W64

# 链接 libiio 库
LIBS += -llibiio

TARGET = DataAcquisitionSystem
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    iioreceiver.cpp \
    main.cpp \
    mainwindow.cpp \
    mainwindow_ui.cpp \
    tcpreceiver.cpp \
    databuffer.cpp \
    waveformwidget.cpp \
    databasemanager.cpp \
    dataprocessor.cpp \
    historyviewer.cpp \
    dataanalyzer.cpp

HEADERS += \
    iioreceiver.h \
    mainwindow.h \
    mainwindow_ui.h \
    tcpreceiver.h \
    databuffer.h \
    waveformwidget.h \
    databasemanager.h \
    dataprocessor.h \
    historyviewer.h \
    dataanalyzer.h

FORMS += \
    mainwindow.ui \
    historyviewer.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
