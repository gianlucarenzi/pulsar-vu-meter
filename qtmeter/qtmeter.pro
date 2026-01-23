QT += core gui widgets serialport
CONFIG += c++11
TARGET = qtmeter
TEMPLATE = app
SOURCES += main.cpp \
           mainwindow.cpp \
           vumeterwidget.cpp \
           serialworker.cpp \
           demomode.cpp \


HEADERS += mainwindow.h \
           vumeterwidget.h \
           serialworker.h \
           demomode.h \


FORMS += mainwindow.ui
