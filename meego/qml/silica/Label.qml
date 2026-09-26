import QtQuick 1.1
// Silica's Label: a Text with the theme's colour and size, wrapping off.
Text {
    property real leftPadding: 0
    property real rightPadding: 0
    property bool highlighted: false
    // QtQuick 2's Text calls it contentWidth, QtQuick 1.1's paintedWidth.
    property real contentWidth: paintedWidth
    property real contentHeight: paintedHeight
    color: highlighted ? AppTheme.highlightColor : AppTheme.primaryColor
    font.pixelSize: AppTheme.fontSizeMedium
    // Silica fades a truncated line; a QtQuick 1.1 Text can only elide.
    property int truncationMode: 0
    // Silicas Label faerbt Verweise damit; der Text von QtQuick 1.1 kennt
    // linkColor noch nicht (das kam mit Qt 5.1). Die Eigenschaft steht hier
    // trotzdem, damit die Seiten sie wie gewohnt setzen koennen -- sichtbar
    // wird sie nicht, Verweise bleiben in der Farbe des Themas.
    property color linkColor: AppTheme.highlightColor
    x: leftPadding
}
