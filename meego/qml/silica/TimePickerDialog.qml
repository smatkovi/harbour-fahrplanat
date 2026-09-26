import QtQuick 1.1
import com.nokia.meego 1.0
import com.nokia.extras 1.1 as Extras

// Wie DatePickerDialog, nur fuer die Uhrzeit. Der Aufrufer liest hour und
// minute, genau wie auf Sailfish.
Page {
    id: root

    property int hour: 0
    property int minute: 0

    signal accepted()

    Component.onCompleted: dialog.open()

    Extras.TimePickerDialog {
        id: dialog
        titleText: "Uhrzeit"
        acceptButtonText: "Wählen"
        rejectButtonText: "Abbrechen"
        // Stunden und Minuten, 24-Stunden-Anzeige. Die Zahlen stehen hier
        // statt DateTime.Hours & Co., weil DateTime aus dem Plugin von
        // com.nokia.extras kommt: auf dem Geraet gibt es den Typ, im
        // QML-Pruefer nicht, und dieselbe Datei soll beides ueberstehen.
        // (plugins.qmltypes: Hours 1, Minutes 2, TwentyFourHours 2.)
        fields: 1 | 2
        hourMode: 2
        hour: root.hour
        minute: root.minute

        onAccepted: {
            root.hour = dialog.hour
            root.minute = dialog.minute
            root.accepted()
            pageStack.pop()
        }
        onRejected: pageStack.pop()
    }
}
