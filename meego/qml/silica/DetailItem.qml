import QtQuick 1.1

// Silicas DetailItem: eine Zeile aus Beschriftung und Wert, die Beschriftung
// halb so breit wie die Zeile.
Item {
    id: root

    property string label
    property string value
    property alias alignment: wert.horizontalAlignment

    width: parent ? parent.width : 0
    height: Math.max(beschriftung.paintedHeight, wert.paintedHeight) + AppTheme.paddingSmall

    Text {
        id: beschriftung
        anchors { left: parent.left; leftMargin: AppTheme.horizontalPageMargin; top: parent.top }
        width: parent.width * 0.4 - AppTheme.horizontalPageMargin
        text: root.label
        color: AppTheme.secondaryColor
        font.pixelSize: AppTheme.fontSizeSmall
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignRight
    }
    Text {
        id: wert
        anchors {
            left: beschriftung.right; leftMargin: AppTheme.paddingMedium
            right: parent.right; rightMargin: AppTheme.horizontalPageMargin
            top: parent.top
        }
        text: root.value
        color: AppTheme.primaryColor
        font.pixelSize: AppTheme.fontSizeSmall
        wrapMode: Text.WordWrap
    }
}
