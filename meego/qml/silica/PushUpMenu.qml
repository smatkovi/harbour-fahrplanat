import QtQuick 1.1
import com.nokia.meego 1.0 as Meego

// Silicas PushUpMenu -- dasselbe wie PullDownMenu, nur am unteren Rand. Auf
// Harmattan gibt es beide Gesten nicht; die Eintraege haengen deshalb an einem
// Knopf unten rechts.
Item {
    id: root

    property bool busy: false
    property bool _moving: false

    parent: null
    Component.onCompleted: {
        var page = root
        while (page && page.pageStack === undefined)
            page = page.parent
        if (page)
            root.parent = page
        _collect()
    }

    function _collect() {
        if (_moving)
            return
        _moving = true
        var moved = true
        while (moved) {
            moved = false
            var kids = root.children
            for (var i = 0; i < kids.length; i++) {
                if (kids[i] !== menu && kids[i] !== button) {
                    kids[i].parent = column
                    moved = true
                    break
                }
            }
        }
        _moving = false
    }

    onChildrenChanged: _collect()

    anchors { bottom: parent ? parent.bottom : undefined; right: parent ? parent.right : undefined }
    width: AppTheme.itemSizeSmall
    height: AppTheme.itemSizeSmall
    z: 100

    Item {
        id: button
        anchors.fill: parent

        Rectangle {
            anchors.fill: parent
            color: AppTheme.highlightColor
            opacity: area.pressed ? 0.3 : 0
            radius: 4
        }
        Text {
            anchors.centerIn: parent
            text: root.busy ? "…" : "⋮"
            color: AppTheme.highlightColor
            font.pixelSize: AppTheme.fontSizeLarge
        }
        MouseArea {
            id: area
            anchors.fill: parent
            onClicked: menu.open()
        }
    }

    Meego.Menu {
        id: menu
        parent: appWindow

        Column {
            id: column
            anchors { left: parent.left; right: parent.right }
            function closeLayout() { menu.close() }
        }
    }
}
