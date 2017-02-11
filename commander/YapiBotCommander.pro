#-------------------------------------------------
#
# Project created by QtCreator 2014-08-06T10:35:35
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = YapiBotCommander
TEMPLATE = app

CONFIG += qwt
CONFIG += qwtpolar


SOURCES += main.cpp\
        mainwindow.cpp \
    videowidget.cpp \
    touchpad.cpp \
    VideoProcessing.cpp \
    map.cpp \
    graphsdiag.cpp \
    logplot.cpp

HEADERS  += mainwindow.h \
    videowidget.h \
    touchpad.h \
    VideoProcessing.h \
    map.h \
    graphsdiag.h \
    logplot.h

FORMS    += mainwindow.ui \
    graphs.ui

LIBS +=  -lvlc-qt -lvlc-qt-widgets -lavcodec -lavutil -lswscale #-lopencv_core -lopencv_imgproc
LIBS += /usr/local/lib/libopencv_core.so
LIBS += /usr/local/lib/libopencv_imgproc.so
