#-------------------------------------------------
#
# Project created by QtCreator 2016-02-28T15:16:07
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OS_vdi
TEMPLATE = app

QMAKE_CXXFLAGS += -fno-omit-frame-pointer

SOURCES += main.cpp\
        mainwindow.cpp \
    localfilesystem.cpp \
    vdifilesystem.cpp \
    vdimap.cpp \
    mbrdata.cpp \
    ext2superblock.cpp \
    ext2groupdescriptor.cpp \
    vdifunctions.cpp \
    vdifilesystemtreeitem.cpp \
    vdifile.cpp \
    ext2folder.cpp \
    ext2file.cpp \
    ext2filesystemmanager.cpp \
    ext2fsentry.cpp

HEADERS  += mainwindow.h \
    localfilesystem.h \
    vdifile.h \
    vdifilesystem.h \
    vdimap.h \
    mbrdata.h \
    ext2superblock.h \
    ext2groupdescriptor.h \
    vdifunctions.h \
    vdifilesystemtreeitem.h \
    ext2folder.h \
    ext2file.h \
    ext2filesystemmanager.h \
    ext2fsentry.h \
    linuxstat.h

FORMS    += mainwindow.ui

RESOURCES += \
    images.qrc
