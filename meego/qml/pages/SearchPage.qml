import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
import "../components"

ThemedPage {
    id: page
    orientationLock: PageOrientation.Automatic
    property variant fromLocation: settings.lastFrom
    property variant toLocation: settings.lastTo
    property variant viaLocation: ({})
    property bool useNow: true
    property date when: new Date()
    property bool isDeparture: true

    function hasLocation(loc) {
        return loc !== undefined && loc !== null && loc.name !== undefined && loc.name !== ""
    }

    function pickLocation(title, callback) {
        var picker = pageStack.push(Qt.resolvedUrl("LocationPickerPage.qml"), { title: title })
        picker.selected.connect(callback)
    }

    function swapLocations() {
        var tmp = fromLocation
        fromLocation = toLocation
        toLocation = tmp
    }

    function currentWhen() {
        return useNow ? new Date() : when
    }

    function startSearch() {
        if (!hasLocation(fromLocation) || !hasLocation(toLocation)) {
            return
        }
        settings.lastFrom = fromLocation
        settings.lastTo = toLocation
        journeyModel.search(fromLocation, toLocation,
                            hasLocation(viaLocation) ? viaLocation : {},
                            currentWhen(), isDeparture)
        pageStack.push(Qt.resolvedUrl("ResultsPage.qml"))
    }

    function optionsSummary() {
        var parts = [settings.productsSummary(settings.products)]
        if (settings.directOnly) {
            parts.push("nur direkt")
        }
        if (settings.minChangeTime > 0) {
            parts.push("Umsteigen ≥ " + settings.minChangeTime + " min")
        }
        if (settings.wheelchair) {
            parts.push("barrierefrei")
        }
        if (settings.bicycle) {
            parts.push("Fahrrad")
        }
        if (settings.einfachRaus) {
            parts.push("Einfach-Raus-Ticket")
        }
        return parts.join(" · ")
    }

    function pickDate() {
        var dialog = pageStack.push(Qt.resolvedUrl("../silica/DatePickerDialog.qml"), { date: currentWhen() })
        dialog.accepted.connect(function() {
            var d = new Date(currentWhen())
            d.setFullYear(dialog.year, dialog.month - 1, dialog.day)
            when = d
            useNow = false
        })
    }

    function pickTime() {
        var now = currentWhen()
        var dialog = pageStack.push(Qt.resolvedUrl("../silica/TimePickerDialog.qml"), {
                                        hour: now.getHours(),
                                        minute: now.getMinutes()
                                    })
        dialog.accepted.connect(function() {
            var d = new Date(currentWhen())
            d.setHours(dialog.hour, dialog.minute, 0, 0)
            when = d
            useNow = false
        })
    }

    Flickable {
        anchors.fill: parent
        pressDelay: 150
        contentHeight: column.height + AppTheme.paddingLarge

        PullDownMenu {
            MenuItem {
                text: "Über"
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }
            MenuItem {
                text: "Einstellungen"
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }
            MenuItem {
                text: "Störungsmeldungen"
                onClicked: pageStack.push(Qt.resolvedUrl("DisruptionsPage.qml"))
            }
        }

        Column {
            id: column
            width: page.width
            spacing: 0

            PageHeader {
                title: "Verbindung suchen"
                description: journeyModel.hasSearch ? "Zuletzt: " + journeyModel.fromName + " → " + journeyModel.toName : ""
            }

            Item {
                width: parent.width
                height: fromButton.height + toButton.height

                Column {
                    id: locationColumn
                    anchors {
                        left: parent.left
                        right: swapButton.left
                    }
                    ValueButton {
                        id: fromButton
                        width: parent.width
                        label: "Von"
                        value: hasLocation(fromLocation) ? fromLocation.name : "Start wählen"
                        valueColor: hasLocation(fromLocation) ? palette.highlightColor : palette.secondaryHighlightColor
                        onClicked: pickLocation("Von", function(loc) { fromLocation = loc })
                    }
                    ValueButton {
                        id: toButton
                        width: parent.width
                        label: "Nach"
                        value: hasLocation(toLocation) ? toLocation.name : "Ziel wählen"
                        valueColor: hasLocation(toLocation) ? palette.highlightColor : palette.secondaryHighlightColor
                        onClicked: pickLocation("Nach", function(loc) { toLocation = loc })
                    }
                }
                IconButton {
                    id: swapButton
                    anchors {
                        right: parent.right
                        rightMargin: AppTheme.paddingMedium
                        verticalCenter: parent.verticalCenter
                    }
                    source: "image://theme/icon-m-toolbar-shuffle-white"
                    enabled: hasLocation(fromLocation) || hasLocation(toLocation)
                    onClicked: swapLocations()
                }
            }

            Item {
                width: parent.width
                height: viaButton.height
                ValueButton {
                    id: viaButton
                    anchors {
                        left: parent.left
                        right: clearViaButton.visible ? clearViaButton.left : parent.right
                    }
                    label: "Via"
                    value: hasLocation(viaLocation) ? viaLocation.name : "optional"
                    valueColor: hasLocation(viaLocation) ? palette.highlightColor : palette.secondaryColor
                    onClicked: pickLocation("Via", function(loc) { viaLocation = loc })
                }
                IconButton {
                    id: clearViaButton
                    visible: hasLocation(viaLocation)
                    anchors {
                        right: parent.right
                        rightMargin: AppTheme.paddingMedium
                        verticalCenter: parent.verticalCenter
                    }
                    source: "image://theme/icon-m-toolbar-close-white"
                    onClicked: viaLocation = {}
                }
            }

            ComboBox {
                label: "Zeitpunkt"
                currentIndex: isDeparture ? 0 : 1
                menu: ContextMenu {
                    MenuItem { text: "Abfahrt" }
                    MenuItem { text: "Ankunft" }
                }
                onCurrentIndexChanged: {
                    if (currentIndex >= 0) {
                        isDeparture = (currentIndex === 0)
                    }
                }
            }

            Item {
                width: parent.width
                height: dateButton.height

                ValueButton {
                    id: dateButton
                    anchors {
                        left: parent.left
                        right: nowButton.left
                    }
                    label: useNow ? "Jetzt" : Qt.formatDate(when, "ddd, d. MMMM yyyy")
                    value: Qt.formatTime(when, "HH:mm")
                    onClicked: pickDate()
                    onPressAndHold: pickTime()
                }
                IconButton {
                    id: nowButton
                    anchors {
                        right: parent.right
                        rightMargin: AppTheme.paddingMedium
                        verticalCenter: parent.verticalCenter
                    }
                    source: "image://theme/icon-m-common-clock-inverse"
                    onClicked: pickTime()
                }
            }

            Row {
                id: quickButtons
                x: AppTheme.horizontalPageMargin
                width: parent.width - 2 * x
                spacing: AppTheme.paddingMedium
                property int buttonWidth: (width - 2 * spacing) / 3
                Button {
                    text: "Jetzt"
                    enabled: !useNow
                    width: quickButtons.buttonWidth
                    onClicked: {
                        useNow = true
                        when = new Date()
                    }
                }
                Button {
                    text: "+1 h"
                    width: quickButtons.buttonWidth
                    onClicked: {
                        var d = new Date(currentWhen())
                        d.setHours(d.getHours() + 1)
                        when = d
                        useNow = false
                    }
                }
                Button {
                    text: "Morgen"
                    width: quickButtons.buttonWidth
                    onClicked: {
                        var d = new Date(currentWhen())
                        d.setDate(d.getDate() + 1)
                        when = d
                        useNow = false
                    }
                }
            }

            Item { width: 1; height: AppTheme.paddingMedium }

            ValueButton {
                label: "Optionen"
                value: optionsSummary()
                onClicked: pageStack.push(Qt.resolvedUrl("OptionsPage.qml"))
            }

            Item { width: 1; height: AppTheme.paddingLarge }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Verbindungen suchen"
                width: AppTheme.buttonWidthLarge
                enabled: hasLocation(fromLocation) && hasLocation(toLocation)
                onClicked: startSearch()
            }

            Item { width: 1; height: AppTheme.paddingLarge }

            Label {
                visible: journeyModel.hasSearch
                x: AppTheme.horizontalPageMargin
                width: parent.width - 2 * x
                wrapMode: Text.Wrap
                font.pixelSize: AppTheme.fontSizeExtraSmall
                color: palette.secondaryColor
                text: "Zuletzt gesucht: " + journeyModel.fromName + " → " + journeyModel.toName
                      + " (" + journeyModel.whenText + ")"
            }
            BackgroundItem {
                visible: journeyModel.hasSearch
                width: parent.width
                height: AppTheme.itemSizeSmall
                Label {
                    x: AppTheme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Letzte Ergebnisse anzeigen"
                    color: parent.highlighted ? palette.highlightColor : palette.primaryColor
                }
                onClicked: pageStack.push(Qt.resolvedUrl("ResultsPage.qml"))
            }
        }
        VerticalScrollDecorator { }
    }

    Timer {
        // keep the "Jetzt" clock label current
        interval: 30000
        running: page.status === PageStatus.Active && useNow
        repeat: true
        onTriggered: when = new Date()
    }
}
