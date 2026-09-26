import QtQuick 1.1
import fahrplanat.polyline 1.0
import com.nokia.meego 1.0
import "../silica"
import "../components"

// Slippy map (Web Mercator, 256 px OpenStreetMap tiles) with the legs of a
// connection drawn on top. No map library needed; tiles are cached by the
// app's network access manager.
ThemedPage {
    id: page

    property variant legs: []            // leg variants of a connection
    property int focusLeg: -1        // index of the leg to fit and emphasise, -1 = whole connection
    property string title: "Karte"
    property string subtitle: ""

    property int tileSize: 256
    property int minZoom: 3
    property int maxZoom: 18
    property int zoom: 14
    property real centerX: 0         // world pixel coordinates at the current zoom
    property real centerY: 0
    property bool fitted: false
    orientationLock: PageOrientation.Automatic
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
            if (spanX <= width - 2 * AppTheme.itemSizeSmall && spanY <= height - 2 * AppTheme.itemSizeLarge) {
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
    Flickable {
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
            property real originX: page.centerX - width / 2
            property real originY: page.centerY - height / 2
            property int firstTileX: Math.floor(originX / page.tileSize)
            property int firstTileY: Math.floor(originY / page.tileSize)
            property int cols: Math.ceil(width / page.tileSize) + 2
            property int rows: Math.ceil(height / page.tileSize) + 2
            property int tilesPerAxis: Math.pow(2, page.zoom)

            Repeater {
                model: tileLayer.cols * tileLayer.rows
                Image {
                    property int tx: tileLayer.firstTileX + (index % tileLayer.cols)
                    property int ty: tileLayer.firstTileY + Math.floor(index / tileLayer.cols)
                    property int wrappedX: ((tx % tileLayer.tilesPerAxis) + tileLayer.tilesPerAxis) % tileLayer.tilesPerAxis
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
        PolylineItem {
            id: overlay
            anchors.fill: parent

            function legLine(leg, emphasised) {
                var pts = page.legPoints(leg)
                if (pts.length < 2) {
                    return null
                }
                var ox = tileLayer.originX
                var oy = tileLayer.originY
                var flach = []
                for (var i = 0; i < pts.length; ++i) {
                    flach.push(page.lonToX(pts[i].lon, page.zoom) - ox)
                    flach.push(page.latToY(pts[i].lat, page.zoom) - oy)
                }
                var blass = (!leg.isWalk && !emphasised && page.focusLeg >= 0) ? 0.45 : 1.0
                return {
                    "points": flach,
                    "color": leg.isWalk ? "#3a3a3a" : leg.product.color,
                    "width": emphasised ? 6 : 4,
                    "casing": emphasised ? 11 : 8,
                    "alpha": blass
                }
            }

            // Heisst wie Silicas Canvas-Methode, damit die Seite unveraendert
            // "overlay.requestPaint()" rufen kann.
            function requestPaint() {
                var neu = []
                for (var i = 0; i < page.legs.length; ++i) {
                    if (page.focusLeg >= 0 && i === page.focusLeg) {
                        continue
                    }
                    var l = legLine(page.legs[i], page.focusLeg < 0)
                    if (l) {
                        neu.push(l)
                    }
                }
                if (page.focusLeg >= 0 && page.focusLeg < page.legs.length) {
                    var f = legLine(page.legs[page.focusLeg], true)
                    if (f) {
                        neu.push(f)
                    }
                }
                lines = neu
            }

            Component.onCompleted: requestPaint()
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
                property real wx: page.lonToX(modelData.lon, page.zoom)
                property real wy: page.latToY(modelData.lat, page.zoom)
                x: wx - tileLayer.originX - width / 2
                y: wy - tileLayer.originY - height / 2
                width: AppTheme.paddingLarge
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
        height: headerColumn.height + 2 * AppTheme.paddingMedium
        // hint that a pulley menu is available here
        Label {
            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
                bottomMargin: AppTheme.paddingSmall / 2
            }
            text: "▾"
            font.pixelSize: AppTheme.fontSizeTiny
            color: palette.secondaryColor
        }
        color: AppTheme.rgba(page.customTheme ? page.pageBackground : AppTheme.overlayBackgroundColor, 0.85)

        Column {
            id: headerColumn
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: AppTheme.horizontalPageMargin
                rightMargin: AppTheme.horizontalPageMargin
                verticalCenter: parent.verticalCenter
            }
            Label {
                width: parent.width
                text: page.title
                elide: Text.ElideRight
                font.pixelSize: AppTheme.fontSizeLarge
                color: palette.highlightColor
            }
            Label {
                width: parent.width
                visible: text !== ""
                text: page.subtitle
                elide: Text.ElideRight
                font.pixelSize: AppTheme.fontSizeSmall
                color: palette.secondaryColor
            }
        }
    }

    Column {
        anchors {
            right: parent.right
            rightMargin: AppTheme.paddingMedium
            bottom: footer.top
            bottomMargin: AppTheme.paddingLarge
        }
        spacing: AppTheme.paddingSmall
        IconButton {
            source: "image://theme/icon-m-toolbar-add-white"
            enabled: page.zoom < page.maxZoom
            onClicked: page.setZoom(page.zoom + 1, page.width / 2, page.height / 2)
            Rectangle { anchors.fill: parent; z: -1; radius: width / 2; color: AppTheme.rgba("#000000", 0.35) }
        }
        IconButton {
            source: "image://theme/icon-m-toolbar-delete-white"
            enabled: page.zoom > page.minZoom
            onClicked: page.setZoom(page.zoom - 1, page.width / 2, page.height / 2)
            Rectangle { anchors.fill: parent; z: -1; radius: width / 2; color: AppTheme.rgba("#000000", 0.35) }
        }
        IconButton {
            source: "image://theme/icon-m-toolbar-refresh-white"
            onClicked: {
                page.fitted = false
                page.fitToPoints()
            }
            Rectangle { anchors.fill: parent; z: -1; radius: width / 2; color: AppTheme.rgba("#000000", 0.35) }
        }
    }

    Rectangle {
        id: footer
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: footerRow.height + 2 * AppTheme.paddingSmall
        color: AppTheme.rgba(page.customTheme ? page.pageBackground : AppTheme.overlayBackgroundColor, 0.85)

        Row {
            id: footerRow
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: AppTheme.horizontalPageMargin
                rightMargin: AppTheme.horizontalPageMargin
                verticalCenter: parent.verticalCenter
            }
            spacing: AppTheme.paddingMedium

            Label {
                width: parent.width
                text: "© OpenStreetMap-Mitwirkende"
                font.pixelSize: AppTheme.fontSizeTiny
                color: palette.secondaryColor
                elide: Text.ElideRight
            }
        }
    }

    Label {
        visible: maps.lastMessage !== ""
        anchors {
            left: parent.left
            right: parent.right
            bottom: footer.top
            leftMargin: AppTheme.horizontalPageMargin
            rightMargin: AppTheme.itemSizeMedium
            bottomMargin: AppTheme.paddingSmall
        }
        text: maps.lastMessage
        wrapMode: Text.Wrap
        font.pixelSize: AppTheme.fontSizeExtraSmall
        color: palette.highlightColor
        Rectangle { anchors.fill: parent; anchors.margins: -AppTheme.paddingSmall; z: -1; radius: AppTheme.paddingSmall; color: AppTheme.rgba(page.customTheme ? page.pageBackground : AppTheme.overlayBackgroundColor, 0.85) }
    }
}
