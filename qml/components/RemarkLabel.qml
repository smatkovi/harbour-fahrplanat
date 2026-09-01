import QtQuick 2.6
import Sailfish.Silica 1.0

// Small secondary text for HAFAS remarks (attributes, hints)
Label {
    width: parent.width
    wrapMode: Text.Wrap
    textFormat: Text.StyledText
    font.pixelSize: Theme.fontSizeExtraSmall
    color: palette.secondaryColor
    linkColor: palette.highlightColor
    onLinkActivated: Qt.openUrlExternally(link)
}
