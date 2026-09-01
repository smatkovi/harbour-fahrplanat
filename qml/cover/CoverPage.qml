import QtQuick 2.6
import Sailfish.Silica 1.0

CoverBackground {
    id: cover

    Column {
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            leftMargin: Theme.paddingLarge
            rightMargin: Theme.paddingLarge
            topMargin: Theme.paddingLarge
        }
        spacing: Theme.paddingSmall

        Label {
            width: parent.width
            text: "Fahrplan AT"
            font.pixelSize: Theme.fontSizeMedium
            color: Theme.highlightColor
            truncationMode: TruncationMode.Fade
        }
        Label {
            width: parent.width
            visible: journeyModel.hasSearch
            text: journeyModel.fromName
            font.pixelSize: Theme.fontSizeSmall
            truncationMode: TruncationMode.Fade
        }
        Label {
            width: parent.width
            visible: journeyModel.hasSearch
            text: "→ " + journeyModel.toName
            font.pixelSize: Theme.fontSizeSmall
            truncationMode: TruncationMode.Fade
        }
        Label {
            width: parent.width
            visible: journeyModel.summary !== ""
            text: journeyModel.summary
            wrapMode: Text.Wrap
            font.pixelSize: Theme.fontSizeLarge
            color: Theme.primaryColor
        }
        Label {
            width: parent.width
            visible: journeyModel.busy
            text: "Aktualisiere …"
            font.pixelSize: Theme.fontSizeExtraSmall
            color: Theme.secondaryColor
        }
        Label {
            width: parent.width
            visible: !journeyModel.hasSearch
            text: "Keine Suche"
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.secondaryColor
        }
    }

    CoverActionList {
        enabled: journeyModel.hasSearch
        CoverAction {
            iconSource: "image://theme/icon-cover-refresh"
            onTriggered: journeyModel.refresh()
        }
    }
}
