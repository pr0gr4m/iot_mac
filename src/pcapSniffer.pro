#-------------------------------------------------
#
# Project created by QtCreator 2017-12-10T14:08:50
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = pcapSniffer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    interfacewidget.cpp \
    probereqsniffer.cpp \
    qcustomplot.cpp

HEADERS  += mainwindow.h \
    interfacewidget.h \
    probereqsniffer.h \
    qcustomplot.h

FORMS    += mainwindow.ui \
    interfacewidget.ui

LIBS += -lpcap \
    -ltins \
    -lpthread
