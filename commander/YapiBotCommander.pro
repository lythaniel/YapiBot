#-------------------------------------------------
#
# Project created by QtCreator 2014-08-06T10:35:35
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = YapiBotCommander
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    videowidget.cpp \
    touchpad.cpp

HEADERS  += mainwindow.h \
    videowidget.h \
    touchpad.h

FORMS    += mainwindow.ui

LIBS += -lvlc-qt -lvlc-qt-widgets -lavcodec -lavutil -lswscale
