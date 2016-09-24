#-------------------------------------------------
#
# Project created by QtCreator 2016-09-24T15:09:33
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MeshViewer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    openglwindow.cpp

HEADERS  += mainwindow.h \
    openglwindow.h \
    common.h

FORMS    += mainwindow.ui

INCLUDEPATH += ./3rdparty/glew-2.0.0/include/

LIBS += -L./3rdparty/glew-2.0.0/build/lib -lGLEW
