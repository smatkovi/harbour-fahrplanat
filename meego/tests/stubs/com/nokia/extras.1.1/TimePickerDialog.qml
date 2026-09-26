// Attrappe fuer den QML-Pruefer; siehe DatePickerDialog.qml.
import QtQuick 1.1
Item {
    property int hour: 0
    property int minute: 0
    property int second: 0
    property int fields: 3
    property int hourMode: 2
    property string acceptButtonText
    property string rejectButtonText
    property string titleText
    signal accepted
    signal rejected
    function open() {}
    function close() {}
}
