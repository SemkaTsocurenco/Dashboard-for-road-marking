import QtQuick
import Theme 1.0

Item {
    id: root

    anchors.right: parent.right
    anchors.bottom: parent.bottom
    anchors.margins: Theme.spacingXLarge

    // Binding to controller connection state
    property bool dataConnected: appController ? appController.isDataConnected : false
    property bool videoConnected: appController ? appController.isVideoConnected : false

    width: background.width
    height: background.height

    component StatusIndicator: Item {
        id: statusIndicator

        property string label: ""
        property bool connected: false

        property color indicatorColor: connected ? Theme.success : Theme.danger

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
                radius: Theme.radiusSmall
                anchors.verticalCenter: parent.verticalCenter
                color: indicatorColor
                border.color: Theme.shadowColor
                border.width: 1

                Behavior on color {
                    ColorAnimation { duration: sceneConfig.colorAnimationDuration }
                }
            }

            Text {
                text: label
                color: Theme.textPrimary
                font.pixelSize: Theme.fontMedium
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
                opacity: connected ? sceneConfig.connectedOpacity : sceneConfig.disconnectedOpacity
            }
        }
    }

    Rectangle {
        id: background
        radius: Theme.radiusLarge
        color: Theme.bgSurface1
        opacity: Theme.overlayOpacity
        border.color: Theme.border
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
