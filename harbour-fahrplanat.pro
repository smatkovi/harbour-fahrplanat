# The name of your application
TARGET = harbour-fahrplanat

CONFIG += sailfishapp c++11
QT += network dbus

isEmpty(VERSION) {
    VERSION = 0.1.6
}
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

SOURCES += \
    src/main.cpp \
    src/appsettings.cpp \
    src/hafasclient.cpp \
    src/hafasparser.cpp \
    src/hafastypes.cpp \
    src/himmodel.cpp \
    src/journeymodel.cpp \
    src/locationmodel.cpp \
    src/maplauncher.cpp \
    src/networkfactory.cpp \
    src/recentmodel.cpp

HEADERS += \
    src/appsettings.h \
    src/hafasclient.h \
    src/hafasparser.h \
    src/hafastypes.h \
    src/himmodel.h \
    src/journeymodel.h \
    src/locationmodel.h \
    src/maplauncher.h \
    src/networkfactory.h \
    src/recentmodel.h

DISTFILES += \
    qml/harbour-fahrplanat.qml \
    qml/cover/CoverPage.qml \
    qml/pages/SearchPage.qml \
    qml/pages/LocationPickerPage.qml \
    qml/pages/ResultsPage.qml \
    qml/pages/JourneyDetailPage.qml \
    qml/pages/MapPage.qml \
    qml/pages/DisruptionsPage.qml \
    qml/pages/DisruptionDetailPage.qml \
    qml/pages/OptionsPage.qml \
    qml/pages/SettingsPage.qml \
    qml/pages/AboutPage.qml \
    qml/components/LineBadge.qml \
    qml/components/RemarkLabel.qml \
    qml/components/TimeLabel.qml \
    qml/components/ThemedPage.qml \
    qml/components/WarningItem.qml \
    qml/icons/train.svg \
    qml/icons/sbahn.svg \
    qml/icons/ubahn.svg \
    qml/icons/tram.svg \
    qml/icons/bus.svg \
    qml/icons/ship.svg \
    qml/icons/taxi.svg \
    qml/icons/walk.svg \
    rpm/harbour-fahrplanat.changes \
    rpm/harbour-fahrplanat.spec \
    harbour-fahrplanat.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172
