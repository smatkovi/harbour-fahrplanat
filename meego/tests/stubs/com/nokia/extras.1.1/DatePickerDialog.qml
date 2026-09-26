// Attrappe fuer den QML-Pruefer; die Eigenschaften sind die der echten
// Komponente (QtSDK .../imports/com/nokia/extras.1.1/DatePickerDialog.qml).
import QtQuick 1.1
Item {
    property int year: 0
    property int month: 1
    property int day: 1
    property int minimumYear: 0
    property int maximumYear: 0
    property string acceptButtonText
    property string rejectButtonText
    property string titleText
    signal accepted
    signal rejected
    function open() {}
    function close() {}
}
