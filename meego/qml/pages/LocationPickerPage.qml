import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
import "../components"

ThemedPage {
    id: page

    property string title: "Ort wählen"
    signal selected(variant location)
    orientationLock: PageOrientation.Automatic
    property bool searching: searchField.text.trim().length >= 2

    function choose(loc) {
        recentModel.add(loc)
        selected(loc)
        pageStack.pop()
    }

    onStatusChanged: {
        if (status === PageStatus.Active) {
            searchField.forceActiveFocus()
        }
    }

    Column {
        id: headerColumn
        width: parent.width

        PageHeader { title: page.title }

        SearchField {
            id: searchField
            width: parent.width
            placeholderText: "Haltestelle, Adresse oder Ort"
            inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
            Keys.onReturnPressed: focus = false
            onTextChanged: locationModel.search(text)
        }
    }

    ListView {
        id: list

        anchors {
            top: headerColumn.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        clip: true
        currentIndex: -1
        model: searching ? locationModel : recentModel

        header: SectionHeader {
            text: searching ? "Suchergebnisse" : "Zuletzt verwendet"
        }

        PullDownMenu {
            visible: !searching && recentModel.count > 0
            MenuItem {
                text: "Verlauf löschen"
                onClicked: recentModel.clearHistory()
            }
        }

        delegate: ListItem {
            id: item

            contentHeight: AppTheme.itemSizeMedium
            menu: searching ? null : recentMenu

            Component {
                id: recentMenu
                ContextMenu {
                    MenuItem {
                        text: model.favorite ? "Favorit entfernen" : "Als Favorit markieren"
                        onClicked: recentModel.toggleFavorite(index)
                    }
                    MenuItem {
                        text: "Aus dem Verlauf entfernen"
                        onClicked: recentModel.remove(index)
                    }
                }
            }

            Image {
                id: favIcon
                visible: !searching && model.favorite === true
                anchors {
                    right: parent.right
                    rightMargin: AppTheme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                source: "image://theme/icon-m-favorite-selected?" + (item.highlighted ? item.palette.highlightColor : item.palette.primaryColor)
                width: AppTheme.iconSizeSmall
                height: width
            }
            Column {
                anchors {
                    left: parent.left
                    right: favIcon.visible ? favIcon.left : parent.right
                    leftMargin: AppTheme.horizontalPageMargin
                    rightMargin: AppTheme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                Label {
                    width: parent.width
                    text: name
                    elide: Text.ElideRight
                    color: item.highlighted ? palette.highlightColor : palette.primaryColor
                }
                Label {
                    width: parent.width
                    text: description
                    visible: text !== ""
                    elide: Text.ElideRight
                    font.pixelSize: AppTheme.fontSizeExtraSmall
                    color: item.highlighted ? palette.secondaryHighlightColor : palette.secondaryColor
                }
            }
            onClicked: choose(location)
        }

        ViewPlaceholder {
            enabled: list.count === 0 && !locationModel.busy
            text: searching
                  ? (locationModel.error !== "" ? locationModel.error : "Keine Treffer")
                  : "Noch kein Verlauf"
            hintText: searching ? "" : "Mindestens zwei Zeichen eingeben"
        }

        BusyIndicator {
            anchors.centerIn: parent
            running: searching && locationModel.busy && list.count === 0
        }

        VerticalScrollDecorator { }
    }
}
