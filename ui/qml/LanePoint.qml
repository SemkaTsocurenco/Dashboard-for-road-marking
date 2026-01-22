import QtQuick
import QtQuick3D

Node {
    id: root

    required property real xMeters
    required property real yMeters
    required property color pointColor
    property real radius: 0.3
    property real height: 0.05
    property bool showLabel: true

    // Coordinates text for label (in meters)
    readonly property string coordText: "X:" + (-xMeters).toFixed(1) + " Z:" + yMeters.toFixed(1)

    // Position the node
    position: Qt.vector3d(
        xMeters * sceneConfig.scaleFactor,
        0,
        yMeters * sceneConfig.scaleFactor
    )

    // Outer cylinder - main point with strong glow
    Model {
        id: outer
        source: "#Cylinder"

        position: Qt.vector3d(0, height / 2, 0)
        scale: Qt.vector3d(radius, height, radius)

        materials: PrincipledMaterial {
            baseColor: pointColor
            emissiveFactor: Qt.vector3d(
                Math.min(1.0, pointColor.r * 0.85),
                Math.min(1.0, pointColor.g * 0.85),
                Math.min(1.0, pointColor.b * 0.85)
            )
            roughness: 0.15
            metalness: 0.0
            specularAmount: 0.4
        }
    }

    // Inner cylinder - accent color (cyan instead of purple for modern look)
    Model {
        id: inner
        source: "#Cylinder"

        position: Qt.vector3d(0, 1 + height / 2, 0)
        scale: Qt.vector3d(radius / 2, height * 1.2, radius / 2)

        materials: PrincipledMaterial {
            baseColor: '#000000'
            emissiveFactor: Qt.vector3d(0.25, 0.75, 0.8)
            roughness: 0.1
            metalness: 0.0
            specularAmount: 0.5
        }
    }

    // Coordinate label above the point
    Model {
        id: labelPlane
        visible: root.showLabel
        source: "#Rectangle"
        position: Qt.vector3d(0, 60, 0)
        scale: Qt.vector3d(4.5, 1.5, 1)
        eulerRotation: Qt.vector3d(-45, 180, 0)

        materials: DefaultMaterial {
            diffuseMap: Texture {
                sourceItem: Rectangle {
                    width: 140
                    height: 36
                    color: "#cc1a1f25"
                    radius: 6
                    border.color: "#39b9c6"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: root.coordText
                        color: "#e6e9ed"
                        font.pixelSize: 14
                        font.bold: true
                        font.family: "monospace"
                    }
                }
            }
            lighting: DefaultMaterial.NoLighting
        }
    }
}
