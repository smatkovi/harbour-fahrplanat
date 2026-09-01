import QtQuick 2.6
import Sailfish.Silica 1.0
import "../components"

ThemedPage {
    id: page

    allowedOrientations: Orientation.All

    Component.onCompleted: {
        if (!himModel.loaded && !himModel.busy) {
            himModel.load()
        }
    }

    SilicaListView {
        id: list

        anchors.fill: parent
        model: himModel
        currentIndex: -1

        header: Column {
            width: list.width
            PageHeader {
                title: "Störungsmeldungen"
                description: himModel.lastUpdate !== "" ? "Stand " + himModel.lastUpdate + " · " + himModel.count + " Meldungen" : ""
            }
            ComboBox {
                label: "Sortierung"
                currentIndex: himModel.sortMode
                menu: ContextMenu {
                    MenuItem { text: "Priorität" }
                    MenuItem { text: "Aktualität" }
                    MenuItem { text: "Titel" }
                }
                property bool ready: false
                Component.onCompleted: ready = true
                onCurrentIndexChanged: {
                    if (ready && currentIndex >= 0) {
                        himModel.sortMode = currentIndex
                    }
                }
            }
        }

        PullDownMenu {
            busy: himModel.busy
            MenuItem {
                text: "Aktualisieren"
                enabled: !himModel.busy
                onClicked: himModel.load()
            }
        }

        delegate: ListItem {
            id: item

            contentHeight: col.height + 2 * Theme.paddingMedium

            Column {
                id: col
                anchors {
                    left: parent.left
                    right: parent.right
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                Label {
                    width: parent.width
                    text: head
                    wrapMode: Text.Wrap
                    color: item.highlighted ? palette.highlightColor : palette.primaryColor
                }
                Label {
                    width: parent.width
                    visible: text !== "" && text !== head
                    text: lead
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                    font.pixelSize: Theme.fontSizeSmall
                    color: item.highlighted ? palette.secondaryHighlightColor : palette.secondaryColor
                }
                Label {
                    width: parent.width
                    visible: text !== ""
                    font.pixelSize: Theme.fontSizeExtraSmall
                    color: palette.secondaryHighlightColor
                    truncationMode: TruncationMode.Fade
                    text: {
                        var parts = []
                        if (affectedLines !== "") {
                            parts.push(affectedLines)
                        } else if (products !== "" && products !== "Alle Verkehrsmittel") {
                            parts.push(products)
                        }
                        if (company !== "") {
                            parts.push(company)
                        }
                        if (validUntil !== "") {
                            parts.push("bis " + validUntil)
                        }
                        return parts.join(" · ")
                    }
                }
            }
            onClicked: pageStack.push(Qt.resolvedUrl("DisruptionDetailPage.qml"), { warning: himModel.get(index) })
        }

        ViewPlaceholder {
            enabled: !himModel.busy && himModel.count === 0
            text: himModel.error !== "" ? himModel.error : "Keine aktuellen Meldungen"
            hintText: "Zum Aktualisieren nach unten ziehen"
        }

        BusyIndicator {
            anchors.centerIn: parent
            size: BusyIndicatorSize.Large
            running: himModel.busy && himModel.count === 0
        }

        VerticalScrollDecorator { }
    }
}
