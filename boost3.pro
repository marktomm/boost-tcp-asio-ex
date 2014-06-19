#-------------------------------------------------
#
# Project created by QtCreator 2014-06-18T11:00:51
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = boost3
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    SokcetManager.cpp \
    TcpIpPort.cpp

QMAKE_LIBS += \
    -lbz2 \
    -llog4cxx \
    -lboost_thread \
    -lboost_filesystem \
    -lboost_date_time \
    -lboost_program_options \
    -lboost_system \
    -lboost_signals

HEADERS += \
    SocketManager.h \
    TcpIpPort.h
