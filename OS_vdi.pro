#-------------------------------------------------
#
# Project created by QtCreator 2016-02-28T15:16:07
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OS_vdi
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    localfilesystem.cpp \
    vdimanager.cpp \
    vdifile.cpp \
    vdifilesystem.cpp \
    vdimap.cpp

HEADERS  += mainwindow.h \
    localfilesystem.h \
    vdimanager.h \
    vdifile.h \
    vdifilesystem.h \
    vdimap.h

FORMS    += mainwindow.ui
