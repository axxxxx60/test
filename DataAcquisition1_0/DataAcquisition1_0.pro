QT += core gui network sql charts
QT += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# libiio头文件路径
#INCLUDEPATH += $$PWD/libiio/include

# libiio库文件路径
LIBS += -L$$PWD/libiio/Windows-MinGW-W64

# 链接 libiio 库
LIBS += -llibiio

TARGET = DataAcquisitionSystem
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    dataanalyzer.cpp \
    databasemanager.cpp \
    databuffer.cpp \
    dataprocessor.cpp \
    historyviewer.cpp \
    iioreceiver.cpp \
    jsonexporter.cpp \
    main.cpp \
    mainwindow.cpp \
    waveformwidget.cpp

HEADERS += \
    dataanalyzer.h \
    databasemanager.h \
    databuffer.h \
    dataprocessor.h \
    historyviewer.h \
    iioreceiver.h \
    jsonexporter.h \
    libiio/include/iio.h \
    mainwindow.h \
    mainwindow_ui.h \
    waveformwidget.h

FORMS += \
    HistoryViewer.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
