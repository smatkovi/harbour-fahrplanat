import QtQuick 2.6
import Sailfish.Silica 1.0
import "../components"

ThemedPage {
    id: page

    property var journey
    property bool showAllStops: false

    allowedOrientations: Orientation.All

    readonly property int timeColumnWidth: Theme.itemSizeSmall * 1.35
    readonly property int platformColumnWidth: Theme.itemSizeSmall * 0.9

    function openMap(legIndex) {
        var props = { "legs": journey.legs, "focusLeg": legIndex }
        if (legIndex >= 0) {
            var leg = journey.legs[legIndex]
            props.title = leg.origin + " → " + leg.destination
            var parts = [leg.isTransfer ? "Umstieg" : "Fußweg"]
            if (leg.durationText !== "") {
                parts.push(leg.durationText)
            }
            if (leg.distanceText !== "") {
                parts.push(leg.distanceText)
            }
            if (!leg.polyline || leg.polyline.length < 2) {
                parts.push("Luftlinie")
            }
            props.subtitle = parts.join(" · ")
        } else {
            props.title = journey.origin + " → " + journey.destination
            props.subtitle = journey.dateText + " · " + journey.durationText
        }
        pageStack.push(Qt.resolvedUrl("MapPage.qml"), props)
    }

    function changesText(n) {
        if (n === 0) {
            return "direkt"
        }
        return n + (n === 1 ? " Umstieg" : " Umstiege")
    }

    SilicaFlickable {
        id: flickable
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        PullDownMenu {
            MenuItem {
                text: "Verbindung auf der Karte"
                onClicked: openMap(-1)
            }
            MenuItem {
                text: showAllStops ? "Zwischenhalte ausblenden" : "Alle Zwischenhalte anzeigen"
                onClicked: showAllStops = !showAllStops
            }
            MenuItem {
                text: "Störungsmeldungen"
                onClicked: pageStack.push(Qt.resolvedUrl("DisruptionsPage.qml"))
            }
        }

        Column {
            id: column
            width: page.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: journey ? journey.origin + " → " + journey.destination : ""
                description: journey ? journey.dateText + " · " + journey.durationText + " · " + changesText(journey.changes) : ""
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                text: maps.pureMapsInstalled ? "Haltestelle antippen: in Pure Maps zeigen" : "Haltestelle antippen: auf der Karte zeigen"
                font.pixelSize: Theme.fontSizeTiny
                color: palette.secondaryColor
            }

            Label {
                visible: maps.lastMessage !== ""
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                text: maps.lastMessage
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeExtraSmall
                color: palette.highlightColor
            }

            // Overview line
            Row {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                spacing: Theme.paddingLarge
                Column {
                    Label {
                        text: "Abfahrt"
                        font.pixelSize: Theme.fontSizeExtraSmall
                        color: palette.secondaryColor
                    }
                    TimeLabel {
                        scheduled: journey ? journey.depTime : ""
                        realtime: journey ? journey.depRealTime : ""
                        hasRealtime: journey ? journey.depHasRealtime : false
                        delay: journey ? journey.depDelay : 0
                        cancelled: journey ? journey.depCancelled : false
                        fontSize: Theme.fontSizeExtraLarge
                    }
                }
                Column {
                    Label {
                        text: "Ankunft"
                        font.pixelSize: Theme.fontSizeExtraSmall
                        color: palette.secondaryColor
                    }
                    TimeLabel {
                        scheduled: journey ? journey.arrTime : ""
                        realtime: journey ? journey.arrRealTime : ""
                        hasRealtime: journey ? journey.arrHasRealtime : false
                        delay: journey ? journey.arrDelay : 0
                        cancelled: journey ? journey.arrCancelled : false
                        fontSize: Theme.fontSizeExtraLarge
                    }
                }
            }

            Label {
                visible: journey ? journey.cancelled : false
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                text: "Diese Verbindung fällt (teilweise) aus."
                color: Theme.errorColor
                wrapMode: Text.Wrap
            }

            // Connection-level disruption messages
            Column {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                spacing: Theme.paddingSmall
                Repeater {
                    model: journey ? journey.warnings : []
                    WarningItem { warning: modelData }
                }
            }

            // Connection-level remarks
            Column {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                Repeater {
                    model: journey ? journey.remarks : []
                    RemarkLabel { text: modelData.text }
                }
            }

            // Legs
            Repeater {
                id: legRepeater
                model: journey ? journey.legs : []
                delegate: legDelegate
            }
        }
        VerticalScrollDecorator { }
    }

    Component {
        id: legDelegate

        Item {
            id: legItem

            property var leg: modelData
            property bool expanded: page.showAllStops
            readonly property bool isWalk: leg.isWalk
            readonly property var product: leg.product
            readonly property string barColor: isWalk ? page.palette.secondaryColor : product.color
            readonly property bool isFirst: index === 0
            readonly property bool isLast: index === legRepeater.count - 1

            width: column.width
            height: legColumn.height + Theme.paddingMedium

            Connections {
                target: page
                onShowAllStopsChanged: legItem.expanded = page.showAllStops
            }

            Rectangle {
                id: bar
                x: Theme.horizontalPageMargin + Theme.paddingSmall
                width: isWalk ? Theme.paddingSmall / 2 : Theme.paddingSmall
                anchors {
                    top: legColumn.top
                    topMargin: Theme.paddingSmall
                    bottom: legColumn.bottom
                }
                color: barColor
                opacity: isWalk ? 0.5 : 1
                radius: width / 2
            }

            Column {
                id: legColumn
                anchors {
                    left: bar.right
                    leftMargin: Theme.paddingMedium
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                }
                spacing: Theme.paddingSmall

                // ---- Walking / transfer leg -----------------------------------
                BackgroundItem {
                    visible: isWalk
                    width: parent.width
                    height: visible ? walkColumn.height + Theme.paddingMedium : 0
                    contentHeight: height
                    enabled: leg.destinationLocation.hasCoord || leg.originLocation.hasCoord
                    onClicked: openMap(index)
                    Column {
                        id: walkColumn
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width
                        Row {
                            spacing: Theme.paddingSmall
                            Image {
                                source: Qt.resolvedUrl("../icons/walk.svg")
                                sourceSize.width: Theme.iconSizeSmall
                                sourceSize.height: Theme.iconSizeSmall
                                width: Theme.iconSizeSmall
                                height: width
                                opacity: 0.7
                            }
                            Label {
                                anchors.verticalCenter: parent.verticalCenter
                                text: leg.isTransfer ? "Umstieg" : "Fußweg"
                                font.pixelSize: Theme.fontSizeSmall
                                font.bold: true
                                color: palette.secondaryColor
                            }
                        }
                        Label {
                            width: parent.width
                            wrapMode: Text.Wrap
                            font.pixelSize: Theme.fontSizeSmall
                            color: parent.parent.highlighted ? palette.highlightColor : palette.secondaryColor
                            text: {
                                if (!isWalk) {
                                    return ""
                                }
                                var parts = []
                                if (leg.durationText !== "") {
                                    parts.push(leg.durationText)
                                }
                                if (leg.distanceText !== "") {
                                    parts.push(leg.distanceText)
                                }
                                var t = parts.join(" · ")
                                if (isFirst || isLast || leg.origin !== leg.destination) {
                                    t += (t !== "" ? "\n" : "") + leg.origin + " → " + leg.destination
                                }
                                return t
                            }
                        }
                        Row {
                            visible: leg.destinationLocation.hasCoord
                            spacing: Theme.paddingSmall
                            Image {
                                source: "image://theme/icon-m-location?" + page.palette.highlightColor
                                width: Theme.iconSizeSmall
                                height: width
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Label {
                                anchors.verticalCenter: parent.verticalCenter
                                text: leg.polyline.length > 1 ? "Fußweg auf der Karte zeigen" : "Fußweg auf der Karte zeigen (Luftlinie)"
                                font.pixelSize: Theme.fontSizeExtraSmall
                                color: palette.highlightColor
                            }
                        }
                    }
                }

                // ---- Departure (tap: show station on the map) -------------------
                BackgroundItem {
                    visible: !isWalk
                    width: parent.width
                    height: depRow.height + Theme.paddingSmall
                    contentHeight: height
                    enabled: leg.originLocation.hasCoord
                    onClicked: maps.showLocation(leg.origin, leg.originLocation.lat, leg.originLocation.lon)
                    Row {
                        id: depRow
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width
                        spacing: Theme.paddingSmall
                        TimeLabel {
                            width: page.timeColumnWidth
                            scheduled: leg.depTime
                            realtime: leg.depRealTime
                            hasRealtime: leg.depHasRealtime
                            delay: leg.depDelay
                            cancelled: leg.depCancelled
                            fontSize: Theme.fontSizeMedium
                        }
                        Label {
                            width: parent.width - page.timeColumnWidth - page.platformColumnWidth - 2 * parent.spacing
                            text: leg.origin
                            wrapMode: Text.Wrap
                            font.bold: true
                            color: parent.parent.highlighted ? palette.highlightColor : palette.primaryColor
                        }
                        Label {
                            width: page.platformColumnWidth
                            horizontalAlignment: Text.AlignRight
                            text: leg.depPlatform !== "" ? "Gl. " + leg.depPlatform : ""
                            font.pixelSize: Theme.fontSizeSmall
                            color: leg.depPlatformChanged ? Theme.errorColor : palette.secondaryColor
                        }
                    }
                }

                // ---- Product / direction ----------------------------------------
                BackgroundItem {
                    visible: !isWalk
                    width: parent.width
                    height: productRow.height + Theme.paddingMedium
                    contentHeight: height
                    enabled: leg.intermediateStops.length > 0
                    onClicked: expanded = !expanded

                    Row {
                        id: productRow
                        anchors.verticalCenter: parent.verticalCenter
                        x: page.timeColumnWidth + Theme.paddingSmall
                        width: parent.width - x
                        spacing: Theme.paddingMedium

                        LineBadge {
                            id: badge
                            text: leg.lineName
                            badgeColor: product.color
                            textColor: product.fgColor
                            icon: leg.icon
                            strikeout: leg.cancelled
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Column {
                            width: parent.width - badge.width - parent.spacing
                            Label {
                                width: parent.width
                                text: leg.direction !== "" ? "→ " + leg.direction : (product.categoryLong !== "" ? product.categoryLong : "")
                                font.pixelSize: Theme.fontSizeSmall
                                truncationMode: TruncationMode.Fade
                                color: palette.highlightColor
                            }
                            Label {
                                width: parent.width
                                font.pixelSize: Theme.fontSizeExtraSmall
                                color: palette.secondaryColor
                                truncationMode: TruncationMode.Fade
                                text: {
                                    var parts = []
                                    if (leg.durationText !== "") {
                                        parts.push(leg.durationText)
                                    }
                                    var n = leg.intermediateStops.length
                                    if (n > 0) {
                                        parts.push(n + (n === 1 ? " Zwischenhalt" : " Zwischenhalte") + (expanded ? " ▲" : " ▼"))
                                    } else if (leg.stops.length > 0) {
                                        parts.push("ohne Zwischenhalt")
                                    }
                                    if (product.operator !== "" && product.operator !== product.categoryLong) {
                                        parts.push(product.operator)
                                    }
                                    return parts.join(" · ")
                                }
                            }
                        }
                    }
                }

                Label {
                    visible: !isWalk && !leg.reachable
                    width: parent.width
                    text: "Anschluss voraussichtlich nicht erreichbar"
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.errorColor
                    wrapMode: Text.Wrap
                }
                Label {
                    visible: !isWalk && leg.cancelled
                    width: parent.width
                    text: "Fahrt fällt aus"
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.errorColor
                }

                // ---- Intermediate stops --------------------------------------------
                Column {
                    visible: !isWalk && expanded
                    width: parent.width
                    Repeater {
                        model: expanded ? leg.intermediateStops : []
                        Row {
                            width: parent.width
                            spacing: Theme.paddingSmall
                            TimeLabel {
                                width: page.timeColumnWidth
                                scheduled: modelData.depTime !== "" ? modelData.depTime : modelData.arrTime
                                realtime: modelData.depTime !== "" ? modelData.depRealTime : modelData.arrRealTime
                                hasRealtime: modelData.depTime !== "" ? modelData.depHasRealtime : modelData.arrHasRealtime
                                delay: modelData.depTime !== "" ? modelData.depDelay : modelData.arrDelay
                                cancelled: modelData.cancelled
                                fontSize: Theme.fontSizeSmall
                                baseColor: page.palette.secondaryColor
                            }
                            Label {
                                width: parent.width - page.timeColumnWidth - page.platformColumnWidth - 2 * parent.spacing
                                text: modelData.name
                                wrapMode: Text.Wrap
                                font.pixelSize: Theme.fontSizeSmall
                                font.strikeout: modelData.cancelled
                                color: palette.secondaryColor
                            }
                            Label {
                                width: page.platformColumnWidth
                                horizontalAlignment: Text.AlignRight
                                text: modelData.depPlatform !== "" ? "Gl. " + modelData.depPlatform : ""
                                font.pixelSize: Theme.fontSizeExtraSmall
                                color: modelData.depPlatformChanged ? Theme.errorColor : palette.secondaryColor
                            }
                        }
                    }
                }

                // ---- Arrival (tap: show station on the map) ------------------------
                BackgroundItem {
                    visible: !isWalk
                    width: parent.width
                    height: arrRow.height + Theme.paddingSmall
                    contentHeight: height
                    enabled: leg.destinationLocation.hasCoord
                    onClicked: maps.showLocation(leg.destination, leg.destinationLocation.lat, leg.destinationLocation.lon)
                    Row {
                        id: arrRow
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width
                        spacing: Theme.paddingSmall
                        TimeLabel {
                            width: page.timeColumnWidth
                            scheduled: leg.arrTime
                            realtime: leg.arrRealTime
                            hasRealtime: leg.arrHasRealtime
                            delay: leg.arrDelay
                            cancelled: leg.arrCancelled
                            fontSize: Theme.fontSizeMedium
                        }
                        Label {
                            width: parent.width - page.timeColumnWidth - page.platformColumnWidth - 2 * parent.spacing
                            text: leg.destination
                            wrapMode: Text.Wrap
                            font.bold: true
                            color: parent.parent.highlighted ? palette.highlightColor : palette.primaryColor
                        }
                        Label {
                            width: page.platformColumnWidth
                            horizontalAlignment: Text.AlignRight
                            text: leg.arrPlatform !== "" ? "Gl. " + leg.arrPlatform : ""
                            font.pixelSize: Theme.fontSizeSmall
                            color: leg.arrPlatformChanged ? Theme.errorColor : palette.secondaryColor
                        }
                    }
                }

                // ---- Messages ------------------------------------------------------
                Column {
                    width: parent.width
                    spacing: Theme.paddingSmall
                    Repeater {
                        model: leg.warnings
                        WarningItem { warning: modelData }
                    }
                }
                Column {
                    width: parent.width
                    Repeater {
                        model: leg.remarks
                        RemarkLabel { text: modelData.text }
                    }
                }
            }
        }
    }
}
