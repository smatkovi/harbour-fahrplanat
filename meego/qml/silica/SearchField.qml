import QtQuick 1.1

// Silicas SearchField ist ein TextField mit Lupe; hier genuegt das TextField
// des Ports mit dem Symbol davor.
TextField {
    id: root
    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
}
