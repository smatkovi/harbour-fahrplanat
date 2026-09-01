import QtQuick 2.6
import Sailfish.Silica 1.0
import "../components"

ThemedPage {
    id: page

    property string title: "Ort wählen"
    signal selected(var location)

    allowedOrientations: Orientation.All

    readonly property bool searching: searchField.text.trim().length >= 2

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
            EnterKey.iconSource: "image://theme/icon-m-enter-close"
            EnterKey.onClicked: focus = false
            onTextChanged: locationModel.search(text)
        }
    }

    SilicaListView {
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

            contentHeight: Theme.itemSizeMedium
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
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                source: "image://theme/icon-m-favorite-selected?" + (item.highlighted ? item.palette.highlightColor : item.palette.primaryColor)
                width: Theme.iconSizeSmall
                height: width
            }
            Column {
                anchors {
                    left: parent.left
                    right: favIcon.visible ? favIcon.left : parent.right
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                Label {
                    width: parent.width
                    text: name
                    truncationMode: TruncationMode.Fade
                    color: item.highlighted ? palette.highlightColor : palette.primaryColor
                }
                Label {
                    width: parent.width
                    text: description
                    visible: text !== ""
                    truncationMode: TruncationMode.Fade
                    font.pixelSize: Theme.fontSizeExtraSmall
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
            size: BusyIndicatorSize.Large
            running: searching && locationModel.busy && list.count === 0
        }

        VerticalScrollDecorator { }
    }
}
