import QtQuick 1.1

// Silicas ViewPlaceholder: der Text, der in einer leeren Liste steht.
Item {
    id: root

    property bool enabled: false
    property string text
    property string hintText

    anchors.centerIn: parent
    width: parent ? parent.width - 2 * AppTheme.horizontalPageMargin : 0
    height: column.height
    visible: enabled
    opacity: enabled ? 1 : 0

    Column {
        id: column
        width: parent.width
        spacing: AppTheme.paddingMedium

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            text: root.text
            color: AppTheme.highlightColor
            font.pixelSize: AppTheme.fontSizeLarge
        }
        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            visible: root.hintText !== ""
            text: root.hintText
            color: AppTheme.secondaryColor
            font.pixelSize: AppTheme.fontSizeSmall
        }
    }
}
