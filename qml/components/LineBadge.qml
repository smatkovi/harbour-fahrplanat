import QtQuick 2.6
import Sailfish.Silica 1.0

// Coloured product badge with pictogram, e.g. [train] RJX 162
Rectangle {
    id: badge

    property string text
    property string badgeColor: "#555555"
    property string textColor: "#ffffff"
    property string icon                       // pictogram name in qml/icons, e.g. "train"
    property int fontSize: Theme.fontSizeSmall
    property bool strikeout: false

    radius: Theme.paddingSmall / 2
    color: badgeColor
    height: label.implicitHeight + Theme.paddingSmall
    width: row.width + Theme.paddingMedium

    Row {
        id: row
        anchors.centerIn: parent
        spacing: Theme.paddingSmall / 2
        Image {
            visible: badge.icon !== ""
            source: badge.icon !== "" ? Qt.resolvedUrl("../icons/" + badge.icon + ".svg") : ""
            sourceSize.width: label.implicitHeight
            sourceSize.height: label.implicitHeight
            width: label.implicitHeight
            height: width
            anchors.verticalCenter: parent.verticalCenter
            // pictograms are white; tint for light badge colours
            opacity: 0.95
        }
        Label {
            id: label
            anchors.verticalCenter: parent.verticalCenter
            text: badge.text
            color: badge.textColor
            font.pixelSize: badge.fontSize
            font.bold: true
            font.strikeout: badge.strikeout
        }
    }
}
