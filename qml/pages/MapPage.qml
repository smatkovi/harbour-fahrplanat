import QtQuick 2.6
import Sailfish.Silica 1.0
import "../components"

// Slippy map (Web Mercator, 256 px OpenStreetMap tiles) with the legs of a
// connection drawn on top. No map library needed; tiles are cached by the
// app's network access manager.
ThemedPage {
    id: page

    property var legs: []            // leg variants of a connection
    property int focusLeg: -1        // index of the leg to fit and emphasise, -1 = whole connection
    property string title: "Karte"
    property string subtitle: ""

    readonly property int tileSize: 256
    readonly property int minZoom: 3
    readonly property int maxZoom: 18
    property int zoom: 14
    property real centerX: 0         // world pixel coordinates at the current zoom
    property real centerY: 0
    property bool fitted: false

    allowedOrientations: Orientation.All

    // ---------------------------------------------------------- projection
    function worldSize(z) {
        return tileSize * Math.pow(2, z)
    }
    function lonToX(lon, z) {
        return (lon + 180) / 360 * worldSize(z)
    }
    function latToY(lat, z) {
        var r = lat * Math.PI / 180
        return (1 - Math.log(Math.tan(r) + 1 / Math.cos(r)) / Math.PI) / 2 * worldSize(z)
    }

    // ---------------------------------------------------------- geometry
    function legPoints(leg) {
        var pts = []
        var i
        if (leg.polyline && leg.polyline.length > 1) {
            for (i = 0; i < leg.polyline.length; ++i) {
                pts.push(leg.polyline[i])
            }
            return pts
        }
        if (!leg.isWalk && leg.stops && leg.stops.length > 1) {
            for (i = 0; i < leg.stops.length; ++i) {
                var l = leg.stops[i].location
                if (l && l.hasCoord) {
                    pts.push({ "lat": l.lat, "lon": l.lon })
                }
            }
            if (pts.length > 1) {
                return pts
            }
            pts = []
        }
        if (leg.originLocation && leg.originLocation.hasCoord) {
            pts.push({ "lat": leg.originLocation.lat, "lon": leg.originLocation.lon })
        }
        if (leg.destinationLocation && leg.destinationLocation.hasCoord) {
            pts.push({ "lat": leg.destinationLocation.lat, "lon": leg.destinationLocation.lon })
        }
        return pts
    }

    function focusPoints() {
        var pts = []
        for (var i = 0; i < legs.length; ++i) {
            if (focusLeg >= 0 && i !== focusLeg) {
                continue
            }
            pts = pts.concat(legPoints(legs[i]))
        }
        return pts
    }

    function fitToPoints() {
        var pts = focusPoints()
        if (pts.length === 0 || width <= 0 || height <= 0) {
            return
        }
        var minLat = 90, maxLat = -90, minLon = 180, maxLon = -180
        for (var i = 0; i < pts.length; ++i) {
            minLat = Math.min(minLat, pts[i].lat); maxLat = Math.max(maxLat, pts[i].lat)
            minLon = Math.min(minLon, pts[i].lon); maxLon = Math.max(maxLon, pts[i].lon)
        }
        var z = maxZoom
        if (focusLeg < 0 || pts.length > 2) {
            z = maxZoom - 1
        }
        for (; z > minZoom; --z) {
            var spanX = lonToX(maxLon, z) - lonToX(minLon, z)
            var spanY = latToY(minLat, z) - latToY(maxLat, z)
            if (spanX <= width - 2 * Theme.itemSizeSmall && spanY <= height - 2 * Theme.itemSizeLarge) {
                break
            }
        }
        zoom = z
        centerX = (lonToX(minLon, z) + lonToX(maxLon, z)) / 2
        centerY = (latToY(minLat, z) + latToY(maxLat, z)) / 2
        fitted = true
        overlay.requestPaint()
    }

    function setZoom(newZoom, anchorX, anchorY) {
        newZoom = Math.max(minZoom, Math.min(maxZoom, newZoom))
        if (newZoom === zoom) {
            return
        }
        // keep the geographic point under the anchor fixed
        var wx = tileLayer.originX + anchorX
        var wy = tileLayer.originY + anchorY
        var f = Math.pow(2, newZoom - zoom)
        zoom = newZoom
        centerX = wx * f - (anchorX - width / 2)
        centerY = wy * f - (anchorY - height / 2)
        overlay.requestPaint()
    }

    function focusedLeg() {
        if (focusLeg < 0 || focusLeg >= legs.length) {
            return null
        }
        return legs[focusLeg]
    }

    function canHandOver() {
        var leg = focusedLeg()
        return leg !== null && (leg.destinationLocation.hasCoord || leg.originLocation.hasCoord)
    }

    function openInPureMaps() {
        var leg = focusedLeg()
        if (leg === null) {
            return
        }
        if (leg.originLocation.hasCoord && leg.destinationLocation.hasCoord) {
            maps.showRoute(leg.origin, leg.originLocation.lat, leg.originLocation.lon,
                           leg.destination, leg.destinationLocation.lat, leg.destinationLocation.lon,
                           legPoints(leg))
        } else if (leg.destinationLocation.hasCoord) {
            maps.showLocation(leg.destination, leg.destinationLocation.lat, leg.destinationLocation.lon)
        } else {
            maps.showLocation(leg.origin, leg.originLocation.lat, leg.originLocation.lon)
        }
    }

    onWidthChanged: if (!fitted) fitToPoints()
    onHeightChanged: if (!fitted) fitToPoints()
    onCenterXChanged: overlay.requestPaint()
    onCenterYChanged: overlay.requestPaint()

    // ---------------------------------------------------------- map
    Rectangle {
        anchors.fill: parent
        color: "#e8e6e1"   // land colour of the tiles, shown while loading
    }

    // Carries the pulley menu. It covers only the header strip at the top, so
    // dragging the map below is unaffected; the pull-down gesture starts there.
    SilicaFlickable {
        id: pulleyHost
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: header.height
        contentHeight: height + 1   // a pulley needs the view to be pullable
        z: 10

        PullDownMenu {
            id: pulley
            MenuItem {
                text: "In Pure Maps öffnen"
                visible: page.canHandOver()
                onClicked: page.openInPureMaps()
            }
            MenuItem {
                text: page.focusLeg >= 0 ? "Ganze Verbindung anzeigen" : "Ansicht einpassen"
                onClicked: {
                    if (page.focusLeg >= 0) {
                        page.focusLeg = -1
                    }
                    page.fitted = false
                    page.fitToPoints()
                    overlay.requestPaint()
                }
            }
        }
    }

    Item {
        id: mapContent
        anchors.fill: parent
        clip: true
        transform: Scale {
            id: pinchScale
            origin.x: 0
            origin.y: 0
            xScale: 1
            yScale: 1
        }

        Item {
            id: tileLayer
            anchors.fill: parent
            readonly property real originX: page.centerX - width / 2
            readonly property real originY: page.centerY - height / 2
            readonly property int firstTileX: Math.floor(originX / page.tileSize)
            readonly property int firstTileY: Math.floor(originY / page.tileSize)
            readonly property int cols: Math.ceil(width / page.tileSize) + 2
            readonly property int rows: Math.ceil(height / page.tileSize) + 2
            readonly property int tilesPerAxis: Math.pow(2, page.zoom)

            Repeater {
                model: tileLayer.cols * tileLayer.rows
                Image {
                    readonly property int tx: tileLayer.firstTileX + (index % tileLayer.cols)
                    readonly property int ty: tileLayer.firstTileY + Math.floor(index / tileLayer.cols)
                    readonly property int wrappedX: ((tx % tileLayer.tilesPerAxis) + tileLayer.tilesPerAxis) % tileLayer.tilesPerAxis
                    x: tx * page.tileSize - tileLayer.originX
                    y: ty * page.tileSize - tileLayer.originY
                    width: page.tileSize
                    height: page.tileSize
                    visible: ty >= 0 && ty < tileLayer.tilesPerAxis
                    asynchronous: true
                    cache: true
                    smooth: true
                    source: visible ? "https://tile.openstreetmap.org/" + page.zoom + "/" + wrappedX + "/" + ty + ".png" : ""
                }
            }
        }

        // Route overlay, repainted whenever the view moves (requests are coalesced per frame)
        Canvas {
            id: overlay
            anchors.fill: parent
            renderStrategy: Canvas.Cooperative

            function drawLeg(ctx, leg, emphasised) {
                var pts = page.legPoints(leg)
                if (pts.length < 2) {
                    return
                }
                var ox = tileLayer.originX
                var oy = tileLayer.originY
                ctx.beginPath()
                for (var i = 0; i < pts.length; ++i) {
                    var px = page.lonToX(pts[i].lon, page.zoom) - ox
                    var py = page.latToY(pts[i].lat, page.zoom) - oy
                    if (i === 0) {
                        ctx.moveTo(px, py)
                    } else {
                        ctx.lineTo(px, py)
                    }
                }
                ctx.lineCap = "round"
                ctx.lineJoin = "round"
                // white casing for contrast on the map
                ctx.lineWidth = emphasised ? 11 : 8
                ctx.strokeStyle = "rgba(255,255,255,0.9)"
                ctx.stroke()
                ctx.lineWidth = emphasised ? 6 : 4
                ctx.strokeStyle = leg.isWalk ? "#3a3a3a" : leg.product.color
                if (!leg.isWalk && !emphasised && page.focusLeg >= 0) {
                    ctx.globalAlpha = 0.45
                }
                ctx.stroke()
                ctx.globalAlpha = 1.0
            }

            onAvailableChanged: if (available) requestPaint()

            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)
                for (var i = 0; i < page.legs.length; ++i) {
                    if (page.focusLeg >= 0 && i === page.focusLeg) {
                        continue
                    }
                    drawLeg(ctx, page.legs[i], page.focusLeg < 0)
                }
                if (page.focusLeg >= 0 && page.focusLeg < page.legs.length) {
                    drawLeg(ctx, page.legs[page.focusLeg], true)
                }
            }
        }

        // Start and destination markers of the focused leg / connection
        Repeater {
            model: {
                var pts = page.focusPoints()
                if (pts.length === 0) {
                    return []
                }
                return [pts[0], pts[pts.length - 1]]
            }
            Rectangle {
                readonly property real wx: page.lonToX(modelData.lon, page.zoom)
                readonly property real wy: page.latToY(modelData.lat, page.zoom)
                x: wx - tileLayer.originX - width / 2
                y: wy - tileLayer.originY - height / 2
                width: Theme.paddingLarge
                height: width
                radius: width / 2
                color: index === 0 ? "#ffffff" : page.accentColor
                border.color: index === 0 ? "#3a3a3a" : "#ffffff"
                border.width: 3
            }
        }

        PinchArea {
            id: pinchArea
            anchors.fill: parent
            property bool pinching: false

            onPinchStarted: {
                pinching = true
                pinchScale.origin.x = pinch.center.x
                pinchScale.origin.y = pinch.center.y
            }
            onPinchUpdated: {
                var s = Math.max(0.5, Math.min(2.0, pinch.scale))
                pinchScale.xScale = s
                pinchScale.yScale = s
            }
            onPinchFinished: {
                var s = pinchScale.xScale
                pinchScale.xScale = 1
                pinchScale.yScale = 1
                pinching = false
                if (s > 1.35) {
                    page.setZoom(page.zoom + 1, pinch.center.x, pinch.center.y)
                } else if (s < 0.75) {
                    page.setZoom(page.zoom - 1, pinch.center.x, pinch.center.y)
                }
            }

            MouseArea {
                anchors.fill: parent
                property real lastX
                property real lastY
                onPressed: {
                    lastX = mouse.x
                    lastY = mouse.y
                }
                onPositionChanged: {
                    if (!pressed || pinchArea.pinching) {
                        lastX = mouse.x
                        lastY = mouse.y
                        return
                    }
                    page.centerX -= mouse.x - lastX
                    page.centerY -= mouse.y - lastY
                    lastX = mouse.x
                    lastY = mouse.y
                }
                onDoubleClicked: page.setZoom(page.zoom + 1, mouse.x, mouse.y)
            }
        }
    }

    // ---------------------------------------------------------- chrome
    Rectangle {
        id: header
        z: 11
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: headerColumn.height + 2 * Theme.paddingMedium
        // hint that a pulley menu is available here
        Label {
            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
                bottomMargin: Theme.paddingSmall / 2
            }
            text: "▾"
            font.pixelSize: Theme.fontSizeTiny
            color: palette.secondaryColor
        }
        color: Theme.rgba(page.customTheme ? page.pageBackground : Theme.overlayBackgroundColor, 0.85)

        Column {
            id: headerColumn
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: Theme.horizontalPageMargin
                rightMargin: Theme.horizontalPageMargin
                verticalCenter: parent.verticalCenter
            }
            Label {
                width: parent.width
                text: page.title
                truncationMode: TruncationMode.Fade
                font.pixelSize: Theme.fontSizeLarge
                color: palette.highlightColor
            }
            Label {
                width: parent.width
                visible: text !== ""
                text: page.subtitle
                truncationMode: TruncationMode.Fade
                font.pixelSize: Theme.fontSizeSmall
                color: palette.secondaryColor
            }
        }
    }

    Column {
        anchors {
            right: parent.right
            rightMargin: Theme.paddingMedium
            bottom: footer.top
            bottomMargin: Theme.paddingLarge
        }
        spacing: Theme.paddingSmall
        IconButton {
            icon.source: "image://theme/icon-m-add"
            enabled: page.zoom < page.maxZoom
            onClicked: page.setZoom(page.zoom + 1, page.width / 2, page.height / 2)
            Rectangle { anchors.fill: parent; z: -1; radius: width / 2; color: Theme.rgba("#000000", 0.35) }
        }
        IconButton {
            icon.source: "image://theme/icon-m-remove"
            enabled: page.zoom > page.minZoom
            onClicked: page.setZoom(page.zoom - 1, page.width / 2, page.height / 2)
            Rectangle { anchors.fill: parent; z: -1; radius: width / 2; color: Theme.rgba("#000000", 0.35) }
        }
        IconButton {
            icon.source: "image://theme/icon-m-refresh"
            onClicked: {
                page.fitted = false
                page.fitToPoints()
            }
            Rectangle { anchors.fill: parent; z: -1; radius: width / 2; color: Theme.rgba("#000000", 0.35) }
        }
    }

    Rectangle {
        id: footer
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: footerRow.height + 2 * Theme.paddingSmall
        color: Theme.rgba(page.customTheme ? page.pageBackground : Theme.overlayBackgroundColor, 0.85)

        Row {
            id: footerRow
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: Theme.horizontalPageMargin
                rightMargin: Theme.horizontalPageMargin
                verticalCenter: parent.verticalCenter
            }
            spacing: Theme.paddingMedium

            Label {
                width: parent.width
                text: "© OpenStreetMap-Mitwirkende"
                font.pixelSize: Theme.fontSizeTiny
                color: palette.secondaryColor
                truncationMode: TruncationMode.Fade
            }
        }
    }

    Label {
        visible: maps.lastMessage !== ""
        anchors {
            left: parent.left
            right: parent.right
            bottom: footer.top
            leftMargin: Theme.horizontalPageMargin
            rightMargin: Theme.itemSizeMedium
            bottomMargin: Theme.paddingSmall
        }
        text: maps.lastMessage
        wrapMode: Text.Wrap
        font.pixelSize: Theme.fontSizeExtraSmall
        color: palette.highlightColor
        Rectangle { anchors.fill: parent; anchors.margins: -Theme.paddingSmall; z: -1; radius: Theme.paddingSmall; color: Theme.rgba(page.customTheme ? page.pageBackground : Theme.overlayBackgroundColor, 0.85) }
    }
}
