import QtQuick 2.6
import Sailfish.Silica 1.0

// Disruption (HIM) message attached to a connection or a leg
BackgroundItem {
    id: item

    property var warning

    width: parent.width
    height: col.height + Theme.paddingMedium
    contentHeight: height

    Rectangle {
        anchors.fill: parent
        color: Theme.rgba(item.palette.highlightBackgroundColor, 0.15)
        radius: Theme.paddingSmall / 2
    }
    Column {
        id: col
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.paddingMedium
            verticalCenter: parent.verticalCenter
        }
        Label {
            width: parent.width
            text: warning ? warning.head : ""
            wrapMode: Text.Wrap
            font.pixelSize: Theme.fontSizeSmall
            font.bold: true
            color: item.highlighted ? palette.highlightColor : palette.primaryColor
        }
        Label {
            width: parent.width
            visible: text !== ""
            text: warning ? (warning.lead !== "" ? warning.lead : warning.plainText) : ""
            wrapMode: Text.Wrap
            maximumLineCount: 3
            elide: Text.ElideRight
            font.pixelSize: Theme.fontSizeExtraSmall
            color: palette.secondaryColor
        }
    }
    onClicked: pageStack.push(Qt.resolvedUrl("../pages/DisruptionDetailPage.qml"), { warning: item.warning })
}
