import QtQuick

Item {
    id: root

    anchors.left: parent.left
    anchors.bottom: parent.bottom
    anchors.margins: 20

    width: 320
    height: 130

    // Binding to laneViewModel; fall back to zero if not available
    property bool hasData: laneViewModel && laneViewModel.valid
    property real offsetMeters: hasData ? laneViewModel.centerOffsetMeters : 0.0
    property real clampedOffset: Math.max(-1.0, Math.min(1.0, offsetMeters))
    property real normalizedOffset: (clampedOffset + 1.0) / 2.0  // 0..1 across the bar

    property color markerColor: {
        var distance = Math.abs(clampedOffset)
        if (!hasData) {
            return "#7f8c8d"
        } else if (distance < 0.3) {
            return "#2ecc71"
        } else if (distance < 0.6) {
            return "#f39c12"
        } else {
            return "#e74c3c"
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: 10
        color: "#1f1f1f"
        border.color: "#2b2b2b"
        border.width: 1
    }

    Column {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12

        Text {
            text: "Center Offset"
            color: "white"
            font.pixelSize: 16
            font.bold: true
            opacity: 0.9
        }

        Item {
            id: barContainer
            width: parent.width
            height: 28

            Rectangle {
                id: bar
                anchors.fill: parent
                radius: height / 2
                color: "#2b2b2b"
                border.color: "#3b3b3b"
                border.width: 1

                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 2
                    height: parent.height
                    color: "#5a5a5a"
                    opacity: 0.9
                }

                Rectangle {
                    id: marker
                    width: 14
                    height: parent.height + 6
                    radius: 6
                    y: (parent.height - height) / 2
                    x: normalizedOffset * (parent.width - width)
                    color: markerColor
                    border.color: "#0d0d0d"
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
                    color: "#9a9a9a"
                    font.pixelSize: 10
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "0"
                    color: "#9a9a9a"
                    font.pixelSize: 10
                }

                Text {
                    anchors.right: parent.right
                    text: "+1.0 m"
                    color: "#9a9a9a"
                    font.pixelSize: 10
                }
            }
        }

        Row {
            width: parent.width
            spacing: 8

            Text {
                text: hasData ? formattedOffset(offsetMeters) : "N/A"
                color: "white"
                font.pixelSize: 16
                font.bold: true
            }

            Text {
                visible: !hasData
                text: "Waiting for data"
                color: "#bbbbbb"
                font.pixelSize: 12
                opacity: 0.8
            }
        }
    }

    function formattedOffset(val) {
        var sign = val >= 0 ? "+" : ""
        return sign + val.toFixed(2) + " m"
    }
}
