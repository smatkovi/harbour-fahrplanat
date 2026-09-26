import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
// Small secondary text for HAFAS remarks (attributes, hints)
Label {
    width: parent.width
    wrapMode: Text.Wrap
    textFormat: Text.StyledText
    font.pixelSize: AppTheme.fontSizeExtraSmall
    color: palette.secondaryColor
    linkColor: palette.highlightColor
    onLinkActivated: Qt.openUrlExternally(link)
}
