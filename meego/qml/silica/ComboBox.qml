import QtQuick 1.1

// Silicas ComboBox: eine Zeile mit Beschriftung und aktueller Wahl, die eine
// ContextMenu oeffnet.
//
// Die Eintraege kommen nicht hier her, sondern stehen in der Menue-Eigenschaft
// -- genau wie auf Sailfish. Welcher Eintrag gewaehlt wurde, meldet die
// ContextMenu ueber ihr Signal "chosen"; ohne das muesste diese Komponente die
// Kinder des Menues selbst durchsuchen, und ein Repeater legt die erst spaeter
// an.
Item {
    id: root

    property string label
    property int currentIndex: 0
    property variant menu
    property string value: (menu && menu.itemText) ? menu.itemText(currentIndex) : ""

    width: parent ? parent.width : 0
    height: Math.max(AppTheme.itemSizeSmall, column.height + AppTheme.paddingMedium)

    onMenuChanged: {
        if (menu && menu.chosen) {
            menu.chosen.connect(function(index) { root.currentIndex = index })
        }
    }

    Column {
        id: column
        anchors {
            left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter
            leftMargin: AppTheme.horizontalPageMargin; rightMargin: AppTheme.horizontalPageMargin
        }

        Text {
            width: parent.width
            text: root.label
            color: AppTheme.secondaryColor
            font.pixelSize: AppTheme.fontSizeExtraSmall
            elide: Text.ElideRight
        }
        Text {
            width: parent.width
            text: root.value
            color: area.pressed ? AppTheme.highlightColor : AppTheme.primaryColor
            font.pixelSize: AppTheme.fontSizeMedium
            elide: Text.ElideRight
        }
    }

    MouseArea {
        id: area
        anchors.fill: parent
        onClicked: if (root.menu) root.menu.open(root)
    }
}
