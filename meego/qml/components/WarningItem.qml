import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
// Disruption (HIM) message attached to a connection or a leg
BackgroundItem {
    id: item

    property variant warning

    width: parent.width
    height: col.height + AppTheme.paddingMedium
    contentHeight: height

    Rectangle {
        anchors.fill: parent
        color: AppTheme.rgba(item.palette.highlightBackgroundColor, 0.15)
        radius: AppTheme.paddingSmall / 2
    }
    Column {
        id: col
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: AppTheme.paddingMedium
            rightMargin: AppTheme.paddingMedium
            verticalCenter: parent.verticalCenter
        }
        Label {
            width: parent.width
            text: warning ? warning.head : ""
            wrapMode: Text.Wrap
            font.pixelSize: AppTheme.fontSizeSmall
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
            font.pixelSize: AppTheme.fontSizeExtraSmall
            color: palette.secondaryColor
        }
    }
    onClicked: pageStack.push(Qt.resolvedUrl("../pages/DisruptionDetailPage.qml"), { warning: item.warning })
}
