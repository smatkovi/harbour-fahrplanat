import QtQuick 2.6
import Sailfish.Silica 1.0
import "../components"

ThemedPage {
    id: page

    property var warning

    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: page.width
            spacing: Theme.paddingMedium

            PageHeader { title: "Meldung" }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                text: warning ? warning.head : ""
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeLarge
                color: palette.highlightColor
            }

            Label {
                visible: warning && warning.lead !== "" && warning.lead !== warning.head && warning.lead !== warning.plainText
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                text: warning ? warning.lead : ""
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeSmall
                color: palette.secondaryHighlightColor
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                text: warning ? warning.text : ""
                wrapMode: Text.Wrap
                textFormat: Text.StyledText
                linkColor: palette.highlightColor
                color: palette.primaryColor
                onLinkActivated: Qt.openUrlExternally(link)
            }

            Item { width: 1; height: Theme.paddingMedium }

            DetailItem {
                visible: value !== ""
                label: "Betroffene Linien"
                value: warning ? warning.affectedLines : ""
            }
            DetailItem {
                visible: value !== "" && (!warning || warning.affectedLines === "")
                label: "Verkehrsmittel"
                value: warning ? warning.products : ""
            }
            DetailItem {
                visible: value !== ""
                label: "Von"
                value: warning ? warning.fromName : ""
            }
            DetailItem {
                visible: value !== ""
                label: "Bis"
                value: warning ? warning.toName : ""
            }
            DetailItem {
                visible: value !== ""
                label: "Gültig ab"
                value: warning ? warning.validFrom : ""
            }
            DetailItem {
                visible: value !== ""
                label: "Gültig bis"
                value: warning ? warning.validUntil : ""
            }
            DetailItem {
                visible: value !== ""
                label: "Aktualisiert"
                value: warning ? warning.modified : ""
            }
            DetailItem {
                visible: value !== ""
                label: "Quelle"
                value: warning ? warning.company : ""
            }
        }
        VerticalScrollDecorator { }
    }
}
