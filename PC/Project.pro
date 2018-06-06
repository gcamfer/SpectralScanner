#-------------------------------------------------
#
# Project created by QtCreator 2016-03-14T18:44:50
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Project
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ledmenu.cpp \
    startupval.cpp \
    calibrationvalues.cpp \
    lasermenu.cpp \
    globals.cpp \
    analysis.cpp \
    steinhart.cpp

HEADERS  += mainwindow.h \
    ledmenu.h \
    startupval.h \
    calibrationvalues.h \
    lasermenu.h \
    globals.h \
    analysis.h \
    steinhart.h

FORMS    += mainwindow.ui \
    ledmenu.ui \
    startupval.ui \
    calibrationvalues.ui \
    lasermenu.ui \
    analysis.ui \
    steinhart.ui
