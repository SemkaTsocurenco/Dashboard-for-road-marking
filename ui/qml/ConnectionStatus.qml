import QtQuick
import Theme 1.0

Item {
    id: root

    anchors.right: parent.right
    anchors.bottom: parent.bottom
    anchors.margins: Theme.spacingXLarge

    property bool dataConnected: appController ? appController.isDataConnected : false
    property bool videoConnected: appController ? appController.isVideoConnected : false
    property bool expanded: false

    property bool anyDisconnected: !dataConnected || !videoConnected

    width: toggleBtn.width
    height: toggleBtn.height + (expanded ? Theme.spacingSmall + background.height : 0)

    component StatusIndicator: Item {
        id: statusIndicator

        property string label: ""
        property bool connected: false

        property color indicatorColor: connected ? Theme.success : Theme.danger

        implicitWidth: row.implicitWidth + 8
        implicitHeight: 22

        Row {
            id: row
            anchors.fill: parent
            spacing: 8
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right

            Rectangle {
                width: 8
                height: 8
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
                font.pixelSize: Theme.fontSmall
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
                opacity: connected ? sceneConfig.connectedOpacity : sceneConfig.disconnectedOpacity
            }
        }
    }

    // Toggle button (always visible)
    Rectangle {
        id: toggleBtn
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: 80
        height: 24
        radius: Theme.radiusMedium
        color: root.anyDisconnected ? Qt.rgba(0.88, 0.36, 0.36, 0.25) : Theme.bgSurface2
        opacity: Theme.overlayOpacity
        border.color: root.anyDisconnected ? Theme.danger : Theme.border
        border.width: 1

        Behavior on color { ColorAnimation { duration: sceneConfig.colorAnimationDuration } }
        Behavior on border.color { ColorAnimation { duration: sceneConfig.colorAnimationDuration } }

        Row {
            anchors.centerIn: parent
            spacing: 5

            Rectangle {
                width: 6
                height: 6
                radius: 3
                anchors.verticalCenter: parent.verticalCenter
                color: root.anyDisconnected ? Theme.danger : Theme.success

                Behavior on color { ColorAnimation { duration: sceneConfig.colorAnimationDuration } }
            }

            Text {
                text: "Status"
                color: Theme.textPrimary
                font.pixelSize: Theme.fontXSmall
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: root.expanded ? "▲" : "▼"
                color: Theme.textSecondary
                font.pixelSize: Theme.fontXSmall
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.expanded = !root.expanded
            cursorShape: Qt.PointingHandCursor
        }
    }

    // Main panel
    Rectangle {
        id: background
        anchors.bottom: toggleBtn.top
        anchors.bottomMargin: Theme.spacingSmall
        anchors.right: parent.right
        radius: Theme.radiusLarge
        color: Theme.bgSurface1
        opacity: Theme.overlayOpacity
        border.color: Theme.border
        border.width: 1
        width: contentRow.implicitWidth + 24
        height: contentRow.implicitHeight + 16
        visible: root.expanded

        Behavior on opacity {
            NumberAnimation { duration: 150; easing.type: Easing.InOutQuad }
        }

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
