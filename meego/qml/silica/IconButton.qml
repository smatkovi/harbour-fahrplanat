import QtQuick 1.1

// Silicas IconButton. Das Symbol wird hier nicht eingefaerbt -- die
// Blanco-Symbole des Geraets sind schon weiss; die Sailfish-Fassung faerbt,
// weil Silicas Symbole einfarbige Schablonen sind.
Item {
    id: root

    property string icon: ""
    property alias source: image.source
    property bool enabled: true
    property bool down: area.pressed
    signal clicked()

    width: AppTheme.itemSizeSmall
    height: AppTheme.itemSizeSmall
    opacity: enabled ? (area.pressed ? 0.6 : 1.0) : 0.4

    Image {
        id: image
        anchors.centerIn: parent
        width: AppTheme.iconSizeMedium
        height: AppTheme.iconSizeMedium
        fillMode: Image.PreserveAspectFit
        smooth: true
    }

    MouseArea {
        id: area
        anchors.fill: parent
        enabled: root.enabled
        onClicked: root.clicked()
    }
}
