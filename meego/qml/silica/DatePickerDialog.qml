import QtQuick 1.1
import com.nokia.meego 1.0
import com.nokia.extras 1.1 as Extras

// Silicas DatePickerDialog wird auf den Stapel geschoben und meldet sich mit
// accepted(); Harmattan hat einen Datumswaehler, aber als Dialog. Diese Seite
// ist nur die Huelle darum: sie oeffnet den Dialog sofort, uebernimmt die
// Auswahl in dieselben Eigenschaften, die der Aufrufer auf Sailfish liest
// (year, month, day), und raeumt sich danach selbst vom Stapel.
Page {
    id: root

    property date date: new Date()
    property int year: date.getFullYear()
    property int month: date.getMonth() + 1
    property int day: date.getDate()

    signal accepted()

    Component.onCompleted: dialog.open()

    Extras.DatePickerDialog {
        id: dialog
        titleText: "Datum"
        acceptButtonText: "Wählen"
        rejectButtonText: "Abbrechen"
        year: root.year
        month: root.month
        day: root.day

        onAccepted: {
            root.year = dialog.year
            root.month = dialog.month
            root.day = dialog.day
            root.accepted()
            pageStack.pop()
        }
        onRejected: pageStack.pop()
    }
}
