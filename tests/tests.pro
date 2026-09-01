# Desktop test harness (not part of the RPM build)
TEMPLATE = app
TARGET = parsertest
CONFIG += console c++11
CONFIG -= app_bundle
QT += core network dbus gui qml
INCLUDEPATH += ../src
DEFINES += APP_VERSION=\\\"test\\\"
SOURCES += parsertest.cpp \
    ../src/hafastypes.cpp \
    ../src/hafasparser.cpp \
    ../src/hafasclient.cpp \
    ../src/appsettings.cpp \
    ../src/locationmodel.cpp \
    ../src/journeymodel.cpp \
    ../src/himmodel.cpp \
    ../src/recentmodel.cpp \
    ../src/maplauncher.cpp \
    ../src/networkfactory.cpp
HEADERS += ../src/hafastypes.h ../src/hafasparser.h ../src/hafasclient.h ../src/appsettings.h \
    ../src/locationmodel.h ../src/journeymodel.h ../src/himmodel.h ../src/recentmodel.h ../src/maplauncher.h ../src/networkfactory.h
