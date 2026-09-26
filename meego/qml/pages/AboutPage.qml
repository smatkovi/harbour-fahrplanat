import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
import "../components"

ThemedPage {
    id: page
    orientationLock: PageOrientation.Automatic
    Flickable {
        anchors.fill: parent
        pressDelay: 150
        contentHeight: column.height + AppTheme.paddingLarge

        Column {
            id: column
            width: page.width
            spacing: AppTheme.paddingMedium

            PageHeader { title: "Über Fahrplan AT" }

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                source: "/usr/share/icons/hicolor/172x172/apps/harbour-fahrplanat.png"
                width: AppTheme.iconSizeExtraLarge
                height: width
            }

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Fahrplan AT " + appVersion
                font.pixelSize: AppTheme.fontSizeLarge
                color: palette.highlightColor
            }

            Label {
                x: AppTheme.horizontalPageMargin
                width: parent.width - 2 * x
                wrapMode: Text.Wrap
                font.pixelSize: AppTheme.fontSizeSmall
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
                    height: AppTheme.itemSizeSmall
                    Label {
                        x: AppTheme.horizontalPageMargin
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.label
                        color: parent.highlighted ? palette.highlightColor : palette.primaryColor
                    }
                    Label {
                        anchors {
                            right: parent.right
                            rightMargin: AppTheme.horizontalPageMargin
                            verticalCenter: parent.verticalCenter
                        }
                        text: modelData.url.replace("https://", "")
                        font.pixelSize: AppTheme.fontSizeExtraSmall
                        color: palette.secondaryColor
                    }
                    onClicked: Qt.openUrlExternally(modelData.url)
                }
            }

            SectionHeader { text: "Lizenz" }

            Label {
                x: AppTheme.horizontalPageMargin
                width: parent.width - 2 * x
                wrapMode: Text.Wrap
                font.pixelSize: AppTheme.fontSizeExtraSmall
                color: palette.secondaryColor
                text: "GNU General Public License v3. Protokollbeschreibung nach public-transport/hafas-client und "
                      + "schildbach/public-transport-enabler."
            }
        }
        VerticalScrollDecorator { }
    }
}
