import QtQuick
import Theme 1.0

Item {
    id: root

    anchors.top: parent.top
    anchors.left: parent.left
    anchors.margins: Theme.spacingXLarge

    Component.onCompleted: console.log("WarningPanel loaded, model activeCount=" + (warningModel ? warningModel.activeCount : "n/a"))

    property bool expanded: false

    width: sceneConfig.warningPanelWidth
    height: toggleBtn.height + (expanded ? Theme.spacingSmall + container.height : 0)

    function severityColor(sev) {
        if (sev === 2) return Theme.danger    // Critical
        if (sev === 1) return Theme.warning   // Warning
        return Theme.accent                   // Info
    }

    function severityIcon(sev) {
        if (sev === 2) return "!"
        if (sev === 1) return "!"
        return "i"
    }

    // Toggle button (always visible)
    Rectangle {
        id: toggleBtn
        anchors.top: parent.top
        anchors.left: parent.left
        width: parent.width
        height: 24
        radius: Theme.radiusMedium
        color: Theme.bgSurface2
        opacity: Theme.overlayOpacity
        border.color: Theme.border
        border.width: 1

        Row {
            anchors.centerIn: parent
            spacing: 6

            Text {
                text: root.expanded ? "▲" : "▼"
                color: Theme.textSecondary
                font.pixelSize: Theme.fontXSmall
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: "Warnings"
                color: Theme.textPrimary
                font.pixelSize: Theme.fontXSmall
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }

            Rectangle {
                height: 16
                width: 22
                radius: Theme.radiusMedium
                color: (warningModel && warningModel.activeCount > 0) ? Theme.warning : Theme.bgSurface1
                border.color: Theme.border
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    anchors.centerIn: parent
                    text: warningModel ? warningModel.activeCount : 0
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontXSmall
                    font.bold: true
                }
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
        id: container
        anchors.top: toggleBtn.bottom
        anchors.topMargin: Theme.spacingSmall
        anchors.left: parent.left
        width: parent.width
        implicitHeight: column.implicitHeight + 16
        height: implicitHeight
        color: Theme.bgSurface1
        opacity: Theme.overlayOpacity
        radius: Theme.radiusXLarge
        border.color: Theme.border
        border.width: 1
        visible: root.expanded

        Behavior on opacity {
            NumberAnimation { duration: 150; easing.type: Easing.InOutQuad }
        }

        Column {
            id: column
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: Theme.spacingMedium
            spacing: Theme.spacingMedium
            width: parent.width

            Loader {
                id: emptyState
                active: warningModel && warningModel.activeCount === 0
                sourceComponent: Text {
                    text: "No active warnings"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSmall
                    opacity: 0.8
                }
            }

            Column {
                width: parent.width
                spacing: Theme.spacingSmall

                Repeater {
                    model: warningModel

                    delegate: Item {
                        width: parent.width
                        height: visible ? 50 : 0
                        visible: model.isActive

                        Rectangle {
                            anchors.fill: parent
                            radius: Theme.radiusLarge
                            color: severityColor(model.severity)
                            opacity: sceneConfig.overlayOpacity

                            border.color: Theme.shadowColor
                            border.width: 1

                            Row {
                                anchors.fill: parent
                                anchors.margins: Theme.spacingMedium
                                spacing: Theme.spacingMedium

                                Rectangle {
                                    width: 20
                                    height: 20
                                    radius: Theme.radiusMedium
                                    color: Theme.bgSurface1
                                    border.color: Theme.border

                                    Text {
                                        anchors.centerIn: parent
                                        text: severityIcon(model.severity)
                                        color: Theme.textPrimary
                                        font.pixelSize: Theme.fontSmall
                                        font.bold: true
                                    }
                                }

                                Column {
                                    width: parent.width - 120
                                    spacing: Theme.spacingXSmall
                                    anchors.verticalCenter: parent.verticalCenter

                                    Text {
                                        text: model.typeName
                                        color: Theme.textPrimary
                                        font.pixelSize: Theme.fontSmall
                                        font.bold: true
                                        elide: Text.ElideRight
                                        width: parent.width
                                    }

                                    Text {
                                        text: model.message
                                        color: Theme.textPrimary
                                        font.pixelSize: Theme.fontXSmall
                                        opacity: 0.9
                                        elide: Text.ElideRight
                                        width: parent.width
                                    }
                                }

                                Column {
                                    spacing: Theme.spacingSmall
                                    anchors.verticalCenter: parent.verticalCenter

                                    Rectangle {
                                        radius: Theme.radiusSmall
                                        color: Theme.bgSurface1
                                        border.color: Theme.border
                                        height: 16
                                        width: 56

                                        Text {
                                            anchors.centerIn: parent
                                            text: (model.distanceMeters !== undefined ? model.distanceMeters.toFixed(1) : "--") + " m"
                                            color: Theme.textPrimary
                                            font.pixelSize: Theme.fontXSmall
                                        }
                                    }

                                    Rectangle {
                                        radius: Theme.radiusSmall
                                        color: Theme.bgSurface1
                                        border.color: Theme.border
                                        height: 16
                                        width: 56

                                        Text {
                                            anchors.centerIn: parent
                                            text: model.severityName || "Severity"
                                            color: Theme.textPrimary
                                            font.pixelSize: Theme.fontXSmall
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
