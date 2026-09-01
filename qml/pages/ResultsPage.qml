import QtQuick 2.6
import Sailfish.Silica 1.0
import "../components"

ThemedPage {
    id: page

    allowedOrientations: Orientation.All

    SilicaListView {
        id: list

        anchors.fill: parent
        model: journeyModel
        currentIndex: -1

        header: Column {
            width: list.width

            PageHeader {
                title: journeyModel.fromName + " → " + journeyModel.toName
                description: journeyModel.whenText
                             + (journeyModel.viaName !== "" ? " · via " + journeyModel.viaName : "")
            }
            BackgroundItem {
                width: parent.width
                height: Theme.itemSizeSmall
                visible: journeyModel.canScrollEarlier && journeyModel.count > 0
                enabled: !journeyModel.busy
                Label {
                    anchors.centerIn: parent
                    text: "Frühere Verbindungen"
                    color: parent.highlighted ? palette.highlightColor : palette.secondaryHighlightColor
                }
                onClicked: journeyModel.searchEarlier()
            }
        }

        footer: Column {
            width: list.width
            BackgroundItem {
                width: parent.width
                height: Theme.itemSizeSmall
                visible: journeyModel.canScrollLater && journeyModel.count > 0
                enabled: !journeyModel.busy
                Label {
                    anchors.centerIn: parent
                    text: "Spätere Verbindungen"
                    color: parent.highlighted ? palette.highlightColor : palette.secondaryHighlightColor
                }
                onClicked: journeyModel.searchLater()
            }
            Label {
                visible: journeyModel.count > 0 && journeyModel.error !== ""
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                wrapMode: Text.Wrap
                text: journeyModel.error
                color: Theme.errorColor
                font.pixelSize: Theme.fontSizeSmall
            }
            Label {
                visible: journeyModel.realtimeUpdated !== "" && journeyModel.count > 0
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                text: "Echtzeitdaten vom Server: " + journeyModel.realtimeUpdated
                font.pixelSize: Theme.fontSizeTiny
                color: palette.secondaryColor
            }
            Item { width: 1; height: Theme.paddingLarge }
        }

        PullDownMenu {
            busy: journeyModel.busy
            MenuItem {
                text: "Neue Suche"
                onClicked: pageStack.pop()
            }
            MenuItem {
                text: "Aktualisieren"
                enabled: !journeyModel.busy
                onClicked: journeyModel.refresh()
            }
            MenuItem {
                text: "Frühere Verbindungen"
                enabled: journeyModel.canScrollEarlier && !journeyModel.busy
                onClicked: journeyModel.searchEarlier()
            }
        }

        PushUpMenu {
            busy: journeyModel.busy
            visible: journeyModel.canScrollLater
            MenuItem {
                text: "Spätere Verbindungen"
                enabled: journeyModel.canScrollLater && !journeyModel.busy
                onClicked: journeyModel.searchLater()
            }
        }

        section {
            property: "dateText"
            delegate: SectionHeader { text: section }
        }

        delegate: ListItem {
            id: item

            contentHeight: content.height + 2 * Theme.paddingMedium

            readonly property int timeColumnWidth: Theme.itemSizeLarge
            readonly property int platformColumnWidth: Theme.itemSizeSmall

            Row {
                id: content
                anchors {
                    left: parent.left
                    right: parent.right
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                spacing: Theme.paddingMedium

                Column {
                    width: item.timeColumnWidth
                    TimeLabel {
                        scheduled: depTime
                        realtime: depRealTime
                        hasRealtime: depHasRealtime
                        delay: depDelay
                        cancelled: model.cancelled
                        fontSize: Theme.fontSizeLarge
                        baseColor: item.highlighted ? item.palette.highlightColor : item.palette.primaryColor
                    }
                    TimeLabel {
                        scheduled: arrTime
                        realtime: arrRealTime
                        hasRealtime: arrHasRealtime
                        delay: arrDelay
                        cancelled: model.cancelled
                        fontSize: Theme.fontSizeMedium
                        baseColor: item.highlighted ? item.palette.secondaryHighlightColor : item.palette.secondaryColor
                    }
                }

                Column {
                    width: content.width - item.timeColumnWidth - item.platformColumnWidth - 2 * content.spacing
                    Label {
                        width: parent.width
                        text: duration + (changes > 0 ? " · " + changes + (changes === 1 ? " Umstieg" : " Umstiege") : " · direkt")
                        font.pixelSize: Theme.fontSizeSmall
                        color: item.highlighted ? palette.secondaryHighlightColor : palette.secondaryColor
                        truncationMode: TruncationMode.Fade
                    }
                    Flow {
                        width: parent.width
                        spacing: Theme.paddingSmall / 2
                        Repeater {
                            model: legsSummary
                            LineBadge {
                                text: modelData.text
                                badgeColor: modelData.color
                                textColor: modelData.fgColor
                                icon: modelData.icon
                                strikeout: modelData.cancelled
                                fontSize: Theme.fontSizeExtraSmall
                                opacity: modelData.isWalk ? 0.75 : 1
                            }
                        }
                    }
                    Label {
                        width: parent.width
                        visible: text !== ""
                        text: model.cancelled ? "Verbindung fällt aus"
                                              : (warningCount > 0 ? warningCount + (warningCount === 1 ? " Meldung" : " Meldungen") : "")
                        font.pixelSize: Theme.fontSizeExtraSmall
                        color: model.cancelled ? Theme.errorColor : palette.highlightColor
                    }
                }

                Column {
                    width: item.platformColumnWidth
                    Label {
                        width: parent.width
                        horizontalAlignment: Text.AlignRight
                        text: depPlatform !== "" ? "Gl. " + depPlatform : ""
                        font.pixelSize: Theme.fontSizeSmall
                        color: depPlatformChanged ? Theme.errorColor
                                                  : (item.highlighted ? palette.secondaryHighlightColor : palette.secondaryColor)
                    }
                }
            }

            onClicked: pageStack.push(Qt.resolvedUrl("JourneyDetailPage.qml"),
                                      { journey: journeyModel.journey(index) })
        }

        ViewPlaceholder {
            enabled: !journeyModel.busy && journeyModel.count === 0
            text: journeyModel.error !== "" ? journeyModel.error : "Keine Verbindungen"
            hintText: "Zum Aktualisieren nach unten ziehen"
        }

        BusyIndicator {
            anchors.centerIn: parent
            size: BusyIndicatorSize.Large
            running: journeyModel.busy && journeyModel.count === 0
        }

        VerticalScrollDecorator { }
    }
}
