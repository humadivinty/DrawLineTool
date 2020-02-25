#-------------------------------------------------
#
# Project created by QtCreator 2020-01-10T09:19:31
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DrawlaneLineTool
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mylabel.cpp \
    GraphBase.cpp \
    cameraModule/BaseCamera.cpp \
    cameraModule/Camera6467_plate.cpp \
    cameraModule/CameraResult.cpp \
    cameraModule/ResultListManager.cpp \
    tinyxml/tinystr.cpp \
    tinyxml/tinyxml.cpp \
    tinyxml/tinyxmlerror.cpp \
    tinyxml/tinyxmlparser.cpp \
    utilityTool/tool_function.cpp \
    utilityTool/encode.c \
    utilityTool/inifile.c

HEADERS  += mainwindow.h \
    mylabel.h \
    GraphBase.h \
    cameraModule/BaseCamera.h \
    cameraModule/Camera6467_plate.h \
    cameraModule/CameraResult.h \
    cameraModule/ResultListManager.h \
    tinyxml/tinystr.h \
    tinyxml/tinyxml.h \
    utilityTool/encode.h \
    utilityTool/inifile.h \
    utilityTool/tool_function.h

FORMS    += mainwindow.ui

unix:!macx|win32: LIBS += -L$$PWD/libHvDevice/ -lHvdevice

unix:!macx|win32: LIBS += -liconv
