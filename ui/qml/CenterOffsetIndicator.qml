import QtQuick
import Theme 1.0

Item {
    id: root

    anchors.left: parent.left
    anchors.bottom: parent.bottom
    anchors.margins: Theme.spacingXLarge

    width: expanded ? sceneConfig.centerOffsetWidth : toggleBtn.width
    height: expanded ? sceneConfig.centerOffsetHeight + toggleBtn.height + Theme.spacingSmall : toggleBtn.height

    property bool expanded: false

    // Binding to laneViewModel; fall back to zero if not available
    property bool hasData: laneViewModel && laneViewModel.valid
    property real offsetMeters: hasData ? laneViewModel.centerOffsetMeters : 0.0
    property real clampedOffset: Math.max(-sceneConfig.maxOffsetM, Math.min(sceneConfig.maxOffsetM, offsetMeters))
    property real normalizedOffset: (clampedOffset + sceneConfig.normalizationDivisor) / sceneConfig.normalizationMultiplier  // 0..1 across the bar

    property color markerColor: {
        var distance = Math.abs(clampedOffset)
        if (!hasData) {
            return Theme.textDisabled
        } else if (distance < sceneConfig.safeThresholdM) {
            return Theme.success
        } else if (distance < sceneConfig.criticalThresholdM) {
            return Theme.warning
        } else {
            return Theme.danger
        }
    }

    // Toggle button (always visible)
    Rectangle {
        id: toggleBtn
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: 90
        height: 24
        radius: Theme.radiusMedium
        color: Theme.bgSurface2
        opacity: Theme.overlayOpacity
        border.color: Theme.border
        border.width: 1

        Row {
            anchors.centerIn: parent
            spacing: 5

            Text {
                text: root.expanded ? "▼" : "▲"
                color: Theme.textSecondary
                font.pixelSize: Theme.fontXSmall
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: "Offset"
                color: Theme.textPrimary
                font.pixelSize: Theme.fontXSmall
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.expanded = !root.expanded
            cursorShape: Qt.PointingHandCursor
        }
    }

    // Main panel (visible only when expanded)
    Item {
        id: panel
        anchors.left: parent.left
        anchors.bottom: toggleBtn.top
        anchors.bottomMargin: Theme.spacingSmall
        width: sceneConfig.centerOffsetWidth
        height: sceneConfig.centerOffsetHeight
        visible: root.expanded
        opacity: root.expanded ? 1.0 : 0.0

        Behavior on opacity {
            NumberAnimation { duration: 150; easing.type: Easing.InOutQuad }
        }

        Rectangle {
            anchors.fill: parent
            radius: Theme.radiusLarge
            color: Theme.bgSurface1
            opacity: Theme.overlayOpacity
            border.color: Theme.border
            border.width: 1
        }

        Column {
            anchors.fill: parent
            anchors.margins: Theme.spacingMedium
            spacing: Theme.spacingMedium

            Text {
                text: "Center Offset"
                color: Theme.textPrimary
                font.pixelSize: Theme.fontMedium
                font.bold: true
                opacity: 0.9
            }

            Item {
                id: barContainer
                width: parent.width
                height: 20

                Rectangle {
                    id: bar
                    anchors.fill: parent
                    radius: height / 2
                    color: Theme.bgSurface2
                    border.color: Theme.border
                    border.width: 1

                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 2
                        height: parent.height
                        color: Theme.textDisabled
                        opacity: 0.9
                    }

                    Rectangle {
                        id: marker
                        width: 14
                        height: parent.height + 6
                        radius: Theme.radiusSmall
                        y: (parent.height - height) / 2
                        x: normalizedOffset * (parent.width - width)
                        color: markerColor
                        border.color: Theme.shadowColor
                        border.width: 1
                        opacity: hasData ? 1.0 : 0.7

                        Behavior on x {
                            NumberAnimation { duration: 200; easing.type: Easing.InOutQuad }
                        }
                    }
                }

                Item {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: -2
                    height: 16

                    Text {
                        anchors.left: parent.left
                        text: "-1.0 m"
                        color: Theme.textSecondary
                        font.pixelSize: Theme.fontXSmall
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "0"
                        color: Theme.textSecondary
                        font.pixelSize: Theme.fontXSmall
                    }

                    Text {
                        anchors.right: parent.right
                        text: "+1.0 m"
                        color: Theme.textSecondary
                        font.pixelSize: Theme.fontXSmall
                    }
                }
            }

            Row {
                width: parent.width
                spacing: Theme.spacingSmall

                Text {
                    text: hasData ? formattedOffset(offsetMeters) : "N/A"
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontMedium
                    font.bold: true
                }

                Text {
                    visible: !hasData
                    text: "Waiting for data"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSmall
                    opacity: 0.8
                }
            }
        }
    }

    function formattedOffset(val) {
        var sign = val >= 0 ? "+" : ""
        return sign + val.toFixed(2) + " m"
    }
}
