import QtQuick 2.6
import Sailfish.Silica 1.0
import "../components"

ThemedPage {
    id: page

    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: page.width
            spacing: Theme.paddingMedium

            PageHeader { title: "Über Fahrplan AT" }

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                source: "/usr/share/icons/hicolor/172x172/apps/harbour-fahrplanat.png"
                width: Theme.iconSizeExtraLarge
                height: width
            }

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Fahrplan AT " + appVersion
                font.pixelSize: Theme.fontSizeLarge
                color: palette.highlightColor
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeSmall
                text: "Fahrplanauskunft für den öffentlichen Verkehr in Österreich mit Echtzeitdaten, "
                      + "Zwischenhalten, Gleisangaben und Störungsmeldungen.\n\n"
                      + "Die Daten kommen von der HAFAS-Schnittstelle unter fahrplan.oebb.at. "
                      + "Diese App ist ein inoffizieller Client und steht in keiner Verbindung zur ÖBB oder zu HaCon."
            }

            SectionHeader { text: "Links" }

            Repeater {
                model: [
                    { label: "Quellcode", url: "https://github.com/smatkovi/harbour-fahrplanat" },
                    { label: "Spende via PayPal", url: "https://www.paypal.me/smatkovi" },
                    { label: "Spende via Liberapay", url: "https://liberapay.com/smatkovi" }
                ]
                BackgroundItem {
                    width: column.width
                    height: Theme.itemSizeSmall
                    Label {
                        x: Theme.horizontalPageMargin
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.label
                        color: parent.highlighted ? palette.highlightColor : palette.primaryColor
                    }
                    Label {
                        anchors {
                            right: parent.right
                            rightMargin: Theme.horizontalPageMargin
                            verticalCenter: parent.verticalCenter
                        }
                        text: modelData.url.replace("https://", "")
                        font.pixelSize: Theme.fontSizeExtraSmall
                        color: palette.secondaryColor
                    }
                    onClicked: Qt.openUrlExternally(modelData.url)
                }
            }

            SectionHeader { text: "Lizenz" }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeExtraSmall
                color: palette.secondaryColor
                text: "GNU General Public License v3. Protokollbeschreibung nach public-transport/hafas-client und "
                      + "schildbach/public-transport-enabler."
            }
        }
        VerticalScrollDecorator { }
    }
}
