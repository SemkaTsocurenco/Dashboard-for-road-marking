import QtQuick
import Theme 1.0

Item {
    id: root

    anchors.top: parent.top
    anchors.left: parent.left
    anchors.margins: Theme.spacingXLarge

    Component.onCompleted: console.log("WarningPanel loaded, model activeCount=" + (warningModel ? warningModel.activeCount : "n/a"))

    width: sceneConfig.warningPanelWidth
    height: container.height

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

    Rectangle {
        id: container
        width: parent.width
        implicitHeight: column.implicitHeight + 28
        height: implicitHeight
        color: Theme.bgSurface1
        opacity: Theme.overlayOpacity
        radius: Theme.radiusXLarge
        border.color: Theme.border
        border.width: 1

        Column {
            id: column
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: Theme.spacingMedium
            spacing: Theme.spacingMedium
            width: parent.width

            Row {
                spacing: Theme.spacingSmall
                anchors.left: parent.left
                anchors.right: parent.right

                Text {
                    text: "Warnings"
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontLarge
                    font.bold: true
                }

                Rectangle {
                    height: 20
                    width: 28
                    radius: Theme.radiusLarge
                    color: Theme.bgSurface2
                    border.color: Theme.border

                    Text {
                        anchors.centerIn: parent
                        text: warningModel ? warningModel.activeCount : 0
                        color: Theme.textPrimary
                        font.pixelSize: Theme.fontSmall
                        font.bold: true
                    }
                }
            }

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
                        height: visible ? 68 : 0
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
                                    width: 28
                                    height: 28
                                    radius: Theme.radiusMedium
                                    color: Theme.bgSurface1
                                    border.color: Theme.border

                                    Text {
                                        anchors.centerIn: parent
                                        text: severityIcon(model.severity)
                                        color: Theme.textPrimary
                                        font.pixelSize: Theme.fontLarge
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
                                        font.pixelSize: Theme.fontMedium
                                        font.bold: true
                                        elide: Text.ElideRight
                                        width: parent.width
                                    }

                                    Text {
                                        text: model.message
                                        color: Theme.textPrimary
                                        font.pixelSize: Theme.fontSmall
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
                                        height: 20
                                        width: 68

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
                                        height: 20
                                        width: 68

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
