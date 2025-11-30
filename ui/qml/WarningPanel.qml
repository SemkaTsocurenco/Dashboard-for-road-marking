import QtQuick

Item {
    id: root

    anchors.top: parent.top
    anchors.left: parent.left
    anchors.margins: 20

    Component.onCompleted: console.log("WarningPanel loaded, model activeCount=" + (warningModel ? warningModel.activeCount : "n/a"))

    width: 360
    height: container.height

    function severityColor(sev) {
        if (sev === 2) return "#e74c3c"   // Critical
        if (sev === 1) return "#f39c12"   // Warning
        return "#3498db"                  // Info
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
        color: "#1f1f1f"
        radius: 12
        border.color: "#2b2b2b"
        border.width: 1

        Column {
            id: column
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 14
            spacing: 12
            width: parent.width

            Row {
                spacing: 8
                anchors.left: parent.left
                anchors.right: parent.right

                Text {
                    text: "Warnings"
                    color: "white"
                    font.pixelSize: 16
                    font.bold: true
                }

                Rectangle {
                    height: 20
                    width: 28
                    radius: 10
                    color: "#2b2b2b"
                    border.color: "#3a3a3a"

                    Text {
                        anchors.centerIn: parent
                        text: warningModel ? warningModel.activeCount : 0
                        color: "#dcdcdc"
                        font.pixelSize: 12
                        font.bold: true
                    }
                }
            }

            Loader {
                id: emptyState
                active: warningModel && warningModel.activeCount === 0
                sourceComponent: Text {
                    text: "No active warnings"
                    color: "#9a9a9a"
                    font.pixelSize: 13
                    opacity: 0.8
                }
            }

            Column {
                width: parent.width
                spacing: 10

                Repeater {
                    model: warningModel

                    delegate: Item {
                        width: parent.width
                        height: visible ? 68 : 0
                        visible: model.isActive

                        Rectangle {
                            anchors.fill: parent
                            radius: 10
                            color: severityColor(model.severity)
                            opacity: 0.92

                            border.color: "#0d0d0d"
                            border.width: 1

                            Row {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 10

                                Rectangle {
                                    width: 28
                                    height: 28
                                    radius: 8
                                    color: "#1f1f1f"
                                    border.color: "#2b2b2b"

                                    Text {
                                        anchors.centerIn: parent
                                        text: severityIcon(model.severity)
                                        color: "white"
                                        font.pixelSize: 16
                                        font.bold: true
                                    }
                                }

                                Column {
                                    width: parent.width - 120
                                    spacing: 4
                                    anchors.verticalCenter: parent.verticalCenter

                                    Text {
                                        text: model.typeName
                                        color: "white"
                                        font.pixelSize: 14
                                        font.bold: true
                                        elide: Text.ElideRight
                                        width: parent.width
                                    }

                                    Text {
                                        text: model.message
                                        color: "white"
                                        font.pixelSize: 12
                                        opacity: 0.9
                                        elide: Text.ElideRight
                                        width: parent.width
                                    }
                                }

                                Column {
                                    spacing: 6
                                    anchors.verticalCenter: parent.verticalCenter

                                    Rectangle {
                                        radius: 6
                                        color: "#1f1f1f"
                                        border.color: "#2b2b2b"
                                        height: 20
                                        width: 68

                                        Text {
                                            anchors.centerIn: parent
                                            text: (model.distanceMeters !== undefined ? model.distanceMeters.toFixed(1) : "--") + " m"
                                            color: "white"
                                            font.pixelSize: 10
                                        }
                                    }

                                    Rectangle {
                                        radius: 6
                                        color: "#1f1f1f"
                                        border.color: "#2b2b2b"
                                        height: 20
                                        width: 68

                                        Text {
                                            anchors.centerIn: parent
                                            text: model.severityName || "Severity"
                                            color: "white"
                                            font.pixelSize: 10
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
