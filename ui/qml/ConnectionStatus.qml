import QtQuick

Item {
    id: root

    anchors.right: parent.right
    anchors.bottom: parent.bottom
    anchors.margins: 20

    // Binding to controller connection state
    property bool dataConnected: appController ? appController.isDataConnected : false
    property bool videoConnected: appController ? appController.isVideoConnected : false

    width: background.width
    height: background.height

    component StatusIndicator: Item {
        id: statusIndicator

        property string label: ""
        property bool connected: false

        property color indicatorColor: connected ? "#2ecc71" : "#e74c3c"

        implicitWidth: row.implicitWidth + 12
        implicitHeight: 28

        Row {
            id: row
            anchors.fill: parent
            spacing: 8
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right

            Rectangle {
                width: 12
                height: 12
                radius: 6
                anchors.verticalCenter: parent.verticalCenter
                color: indicatorColor
                border.color: "#0d0d0d"
                border.width: 1

                Behavior on color {
                    ColorAnimation { duration: sceneConfig.colorAnimationDuration }
                }
            }

            Text {
                text: label
                color: "white"
                font.pixelSize: 14
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
                opacity: connected ? sceneConfig.connectedOpacity : sceneConfig.disconnectedOpacity
            }
        }
    }

    Rectangle {
        id: background
        radius: 10
        color: "#1f1f1f"
        border.color: "#2b2b2b"
        border.width: 1
        width: contentRow.implicitWidth + 24
        height: contentRow.implicitHeight + 16

        Row {
            id: contentRow
            anchors.centerIn: parent
            spacing: 16

            StatusIndicator {
                label: "DATA"
                connected: root.dataConnected
            }

            StatusIndicator {
                label: "VIDEO"
                connected: root.videoConnected
            }
        }
    }
}
