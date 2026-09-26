import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
import "../components"

ThemedPage {
    id: page

    property variant warning
    orientationLock: PageOrientation.Automatic
    Flickable {
        anchors.fill: parent
        pressDelay: 150
        contentHeight: column.height + AppTheme.paddingLarge

        Column {
            id: column
            width: page.width
            spacing: AppTheme.paddingMedium

            PageHeader { title: "Meldung" }

            Label {
                x: AppTheme.horizontalPageMargin
                width: parent.width - 2 * x
                text: warning ? warning.head : ""
                wrapMode: Text.Wrap
                font.pixelSize: AppTheme.fontSizeLarge
                color: palette.highlightColor
            }

            Label {
                visible: warning && warning.lead !== "" && warning.lead !== warning.head && warning.lead !== warning.plainText
                x: AppTheme.horizontalPageMargin
                width: parent.width - 2 * x
                text: warning ? warning.lead : ""
                wrapMode: Text.Wrap
                font.pixelSize: AppTheme.fontSizeSmall
                color: palette.secondaryHighlightColor
            }

            Label {
                x: AppTheme.horizontalPageMargin
                width: parent.width - 2 * x
                text: warning ? warning.text : ""
                wrapMode: Text.Wrap
                textFormat: Text.StyledText
                linkColor: palette.highlightColor
                color: palette.primaryColor
                onLinkActivated: Qt.openUrlExternally(link)
            }

            Item { width: 1; height: AppTheme.paddingMedium }

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
