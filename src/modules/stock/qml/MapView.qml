import QtQuick 2.15
import QtQuick.Controls 2.15
import QtLocation 6.0
import QtPositioning 6.0

Item {
    id: root
    anchors.fill: parent

    signal markerClicked(string localeName)

    property var markersData: []
    property string highlightedLocale: ""   // set from C++ to highlight a searched locale

    onMarkersDataChanged: {
        console.log("QML: markersData updated, count:", markersData ? markersData.length : 0)
    }

    // ── OSM map plugin ────────────────────────────────────────────────────────
    Plugin {
        id: mapPlugin
        name: "osm"
        PluginParameter { name: "osm.mapping.custom.host";                     value: "https://tile.openstreetmap.org/" }
        PluginParameter { name: "osm.mapping.providersrepository.disabled";    value: true }
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        center: QtPositioning.coordinate(33.8869, 9.5375)
        zoomLevel: 6.8
        minimumZoomLevel: 5
        maximumZoomLevel: 18
        copyrightsVisible: false

        // Gesture handlers
        PinchHandler {
            id: pinch
            target: null
            onActiveChanged: if (active) map.startCentroidInertial()
            onScaleChanged: (delta) => {
                map.zoomLevel += Math.log2(delta)
                map.alignCoordinateToPoint(map.toCoordinate(pinch.centroid.position), pinch.centroid.position)
            }
        }
        WheelHandler {
            id: wheel
            acceptedDevices: Qt.platform.pluginName === "cocoa" || Qt.platform.pluginName === "wayland"
                ? PointerDevice.Mouse | PointerDevice.TouchPad
                : PointerDevice.Mouse
            rotationScale: 1.0 / 120.0
            property: "zoomLevel"
        }
        DragHandler {
            id: drag
            target: null
            onTranslationChanged: (delta) => map.pan(-delta.x, -delta.y)
        }

        // ── Warehouse markers ─────────────────────────────────────────────────
        MapItemView {
            model: root.markersData

            delegate: MapQuickItem {
                id: markerItem
                coordinate: QtPositioning.coordinate(modelData.lat, modelData.lon)
                anchorPoint.x: bubble.width / 2
                anchorPoint.y: bubble.height + pin.height

                property bool isHighlighted: root.highlightedLocale !== "" &&
                                             modelData.name.toLowerCase().indexOf(root.highlightedLocale.toLowerCase()) >= 0
                property bool hasAlerts: modelData.alertCount > 0
                property bool allAlert:  modelData.alertCount >= modelData.total

                property color baseColor: {
                    if (allAlert)    return "#ef4444"
                    if (hasAlerts)   return "#f97316"
                    return "#8A9A5B"
                }

                sourceItem: Item {
                    width:  bubble.width
                    height: bubble.height + pin.height

                    // ── Pulse ring (alert markers only) ───────────────────────
                    Rectangle {
                        id: pulseRing
                        visible: markerItem.hasAlerts
                        anchors.centerIn: bubble
                        width:  bubble.width
                        height: bubble.height
                        radius: width / 2
                        color:  "transparent"
                        border.color: markerItem.baseColor
                        border.width: 2
                        opacity: 0

                        SequentialAnimation on opacity {
                            running: markerItem.hasAlerts
                            loops: Animation.Infinite
                            NumberAnimation { to: 0.7; duration: 700; easing.type: Easing.OutCubic }
                            NumberAnimation { to: 0.0; duration: 900; easing.type: Easing.InCubic }
                        }
                        SequentialAnimation on scale {
                            running: markerItem.hasAlerts
                            loops: Animation.Infinite
                            NumberAnimation { to: 1.7; duration: 700; easing.type: Easing.OutCubic }
                            NumberAnimation { to: 1.0; duration: 900; easing.type: Easing.InCubic }
                        }
                    }

                    // ── Highlight glow ring ───────────────────────────────────
                    Rectangle {
                        id: glowRing
                        visible: markerItem.isHighlighted
                        anchors.centerIn: bubble
                        width:  bubble.width + 12
                        height: bubble.height + 12
                        radius: width / 2
                        color:  "transparent"
                        border.color: "#facc15"
                        border.width: 3
                        opacity: 0.9
                    }

                    // ── Main bubble ───────────────────────────────────────────
                    Rectangle {
                        id: bubble
                        width:  44
                        height: 44
                        radius: 22
                        color:  markerItem.baseColor
                        border.color: "#ffffff"
                        border.width: 2.5

                        layer.enabled: true
                        layer.effect: null   // no QtGraphicalEffects dependency

                        scale: tapHandler.pressed ? 0.85 : (markerItem.isHighlighted ? 1.15 : 1.0)
                        Behavior on scale { NumberAnimation { duration: 100; easing.type: Easing.OutBack } }

                        // Material count text
                        Text {
                            anchors.centerIn: parent
                            text:  modelData.total
                            color: "#ffffff"
                            font.pixelSize: 14
                            font.bold: true
                            font.letterSpacing: -0.5
                        }

                        // Alert badge (top-right corner)
                        Rectangle {
                            visible: markerItem.hasAlerts
                            anchors.top:   parent.top
                            anchors.right: parent.right
                            anchors.topMargin:   -2
                            anchors.rightMargin: -2
                            width:  18
                            height: 18
                            radius: 9
                            color:  "#ef4444"
                            border.color: "#ffffff"
                            border.width: 1.5

                            Text {
                                anchors.centerIn: parent
                                text: modelData.alertCount > 9 ? "9+" : modelData.alertCount
                                color: "#ffffff"
                                font.pixelSize: 8
                                font.bold: true
                            }
                        }

                        TapHandler {
                            id: tapHandler
                            onTapped: {
                                root.markerClicked(modelData.name)
                                flyAnimation.start()
                            }
                        }

                        HoverHandler { id: hoverHandler }

                        // Click ripple
                        Rectangle {
                            id: ripple
                            anchors.centerIn: parent
                            width:  0
                            height: 0
                            radius: width / 2
                            color:  "transparent"
                            border.color: "#ffffff"
                            border.width: 1.5
                            opacity: 0

                            ParallelAnimation {
                                id: flyAnimation
                                NumberAnimation { target: ripple; property: "width";   from: 0; to: 60; duration: 350; easing.type: Easing.OutCubic }
                                NumberAnimation { target: ripple; property: "height";  from: 0; to: 60; duration: 350; easing.type: Easing.OutCubic }
                                NumberAnimation { target: ripple; property: "opacity"; from: 0.8; to: 0; duration: 350 }
                            }
                        }
                    }

                    // ── Pin triangle ──────────────────────────────────────────
                    Canvas {
                        id: pin
                        width:  16
                        height: 9
                        anchors.horizontalCenter: bubble.horizontalCenter
                        anchors.top: bubble.bottom
                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.clearRect(0, 0, width, height)
                            ctx.fillStyle = markerItem.baseColor
                            ctx.beginPath()
                            ctx.moveTo(0, 0)
                            ctx.lineTo(width, 0)
                            ctx.lineTo(width / 2, height)
                            ctx.closePath()
                            ctx.fill()
                        }
                        Connections {
                            target: markerItem
                            function onBaseColorChanged() { pin.requestPaint() }
                        }
                    }

                    // ── Rich tooltip ──────────────────────────────────────────
                    Rectangle {
                        id: tooltip
                        visible: hoverHandler.hovered
                        anchors.bottom: bubble.top
                        anchors.horizontalCenter: bubble.horizontalCenter
                        anchors.bottomMargin: 8
                        width:  tooltipContent.implicitWidth + 20
                        height: tooltipContent.implicitHeight + 14
                        radius: 8
                        color:  "#1a1a1aee"
                        border.color: markerItem.baseColor
                        border.width: 1

                        // Small arrow
                        Canvas {
                            width: 12; height: 6
                            anchors.top:              parent.bottom
                            anchors.horizontalCenter: parent.horizontalCenter
                            onPaint: {
                                var c = getContext("2d")
                                c.clearRect(0,0,width,height)
                                c.fillStyle = "#1a1a1aee"
                                c.beginPath(); c.moveTo(0,0); c.lineTo(width,0); c.lineTo(width/2,height); c.closePath(); c.fill()
                            }
                        }

                        Column {
                            id: tooltipContent
                            anchors.centerIn: parent
                            spacing: 3

                            Text {
                                text:  modelData.name
                                color: "#f9fafb"
                                font.pixelSize: 12
                                font.bold: true
                            }
                            Text {
                                text:  "Materiaux : " + modelData.total
                                color: "#9ca3af"
                                font.pixelSize: 10
                            }
                            Text {
                                visible: markerItem.hasAlerts
                                text:    "Alertes : " + modelData.alertCount
                                color:   "#ef4444"
                                font.pixelSize: 10
                                font.bold: true
                            }
                            Text {
                                visible: !markerItem.hasAlerts
                                text:    "Stock OK"
                                color:   "#8A9A5B"
                                font.pixelSize: 10
                            }
                        }
                    }
                }
            }
        }

        // ── Legend ────────────────────────────────────────────────────────────
        Rectangle {
            id: legendBox
            anchors.left:    parent.left
            anchors.bottom:  parent.bottom
            anchors.margins: 14
            width:  175
            height: legendCol.implicitHeight + 20
            radius: 10
            color:  "#1a1a1acc"
            border.color: "#3a3a3a"
            border.width: 1

            Column {
                id: legendCol
                anchors { left: parent.left; right: parent.right; top: parent.top }
                anchors.margins: 10
                spacing: 6

                Text {
                    text: "Legende"
                    font.bold: true
                    font.pixelSize: 11
                    color: "#f9fafb"
                    bottomPadding: 2
                }

                Row {
                    spacing: 8
                    Rectangle { width:12; height:12; radius:6; color:"#8A9A5B"; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: "Stock OK"; font.pixelSize: 10; color: "#d1d5db" }
                }
                Row {
                    spacing: 8
                    Rectangle { width:12; height:12; radius:6; color:"#f97316"; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: "Alertes partielles"; font.pixelSize: 10; color: "#d1d5db" }
                }
                Row {
                    spacing: 8
                    Rectangle { width:12; height:12; radius:6; color:"#ef4444"; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: "Toutes en alerte"; font.pixelSize: 10; color: "#d1d5db" }
                }
                Row {
                    spacing: 8
                    Rectangle { width:12; height:12; radius:6; color:"transparent"; border.color:"#facc15"; border.width:2; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: "Entrepot recherche"; font.pixelSize: 10; color: "#d1d5db" }
                }
            }
        }

        // ── Zoom controls ─────────────────────────────────────────────────────
        Column {
            anchors.right:   parent.right
            anchors.bottom:  parent.bottom
            anchors.margins: 14
            spacing: 4

            Rectangle {
                width: 34; height: 34; radius: 8
                color: "#1a1a1acc"; border.color: "#3a3a3a"
                Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 20; color: "#f9fafb" }
                TapHandler { onTapped: map.zoomLevel = Math.min(map.zoomLevel + 1, map.maximumZoomLevel) }
            }
            Rectangle {
                width: 34; height: 34; radius: 8
                color: "#1a1a1acc"; border.color: "#3a3a3a"
                Text { anchors.centerIn: parent; text: "\u2212"; font.pixelSize: 22; color: "#f9fafb" }
                TapHandler { onTapped: map.zoomLevel = Math.max(map.zoomLevel - 1, map.minimumZoomLevel) }
            }
            // Reset view
            Rectangle {
                width: 34; height: 34; radius: 8
                color: "#1a1a1acc"; border.color: "#3a3a3a"
                Text { anchors.centerIn: parent; text: "\u25a1"; font.pixelSize: 14; color: "#9ca3af" }
                TapHandler {
                    onTapped: {
                        map.center    = QtPositioning.coordinate(33.8869, 9.5375)
                        map.zoomLevel = 6.8
                    }
                }
                ToolTip { visible: parent.children[2].containsMouse; text: "Recentrer"; delay: 300; font.pixelSize: 11 }
                HoverHandler { id: resetHover }
            }
        }

        // ── Copyright ─────────────────────────────────────────────────────────
        Text {
            anchors.bottom: parent.bottom
            anchors.right:  parent.right
            anchors.margins: 4
            text:  "© OpenStreetMap contributors"
            font.pixelSize: 9
            color: "#6b7280"
        }
    }
}
