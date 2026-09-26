import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
import "../components"

ThemedPage {
    id: page
    orientationLock: PageOrientation.Automatic
    property variant changeTimes: [0, 10, 15, 20, 25, 30, 35, 40, 45]

    function changeTimeIndex(minutes) {
        for (var i = 0; i < changeTimes.length; ++i) {
            if (changeTimes[i] === minutes) {
                return i
            }
        }
        return 0
    }

    Flickable {
        anchors.fill: parent
        pressDelay: 150
        contentHeight: column.height + AppTheme.paddingLarge

        PullDownMenu {
            MenuItem {
                text: "Alle Verkehrsmittel"
                onClicked: settings.products = settings.allProducts()
            }
            MenuItem {
                text: "Nur Züge"
                onClicked: settings.products = 0x101D
            }
            MenuItem {
                text: "Nur Nah-/Regionalverkehr"
                onClicked: settings.products = 0x0BF0
            }
            MenuItem {
                text: "Nur Fernreise-/Nachtzüge"
                onClicked: settings.products = 0x100D
            }
        }

        Column {
            id: column
            width: page.width

            PageHeader {
                title: "Suchoptionen"
                description: settings.productsSummary(settings.products)
            }

            SectionHeader { text: "Verkehrsmittel" }

            Repeater {
                model: settings.productGroupCount()
                TextSwitch {
                    text: settings.productGroupName(index)
                    automaticCheck: false
                    checked: (settings.products & settings.productGroupBits(index)) !== 0
                    onClicked: {
                        var bits = settings.productGroupBits(index)
                        if (checked) {
                            settings.products = settings.products & ~bits
                        } else {
                            settings.products = settings.products | bits
                        }
                    }
                }
            }

            SectionHeader { text: "Verbindung" }

            TextSwitch {
                text: "Nur Direktverbindungen"
                automaticCheck: false
                checked: settings.directOnly
                onClicked: settings.directOnly = !settings.directOnly
            }

            ComboBox {
                property bool ready: false
                label: "Umsteigezeit"
                currentIndex: changeTimeIndex(settings.minChangeTime)
                menu: ContextMenu {
                    Repeater {
                        model: changeTimes
                        MenuItem { text: modelData === 0 ? "Normal" : "mindestens " + modelData + " Minuten" }
                    }
                }
                Component.onCompleted: ready = true
                onCurrentIndexChanged: {
                    if (ready && currentIndex >= 0) {
                        settings.minChangeTime = changeTimes[currentIndex]
                    }
                }
            }

            TextSwitch {
                text: "Barrierefrei (Rollstuhl)"
                description: "Nur Fahrten mit rollstuhlgerechten Fahrzeugen"
                automaticCheck: false
                checked: settings.wheelchair
                onClicked: settings.wheelchair = !settings.wheelchair
            }

            TextSwitch {
                text: "Fahrradmitnahme"
                automaticCheck: false
                checked: settings.bicycle
                onClicked: settings.bicycle = !settings.bicycle
            }

            TextSwitch {
                text: "Einfach-Raus-Ticket"
                description: "Nur Verbindungen mit Regionalzügen, S-Bahnen und Schienenersatzverkehr"
                automaticCheck: false
                checked: settings.einfachRaus
                onClicked: settings.einfachRaus = !settings.einfachRaus
            }
        }
        VerticalScrollDecorator { }
    }
}
