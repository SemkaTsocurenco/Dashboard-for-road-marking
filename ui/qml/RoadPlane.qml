import QtQuick
import QtQuick3D

Node {
    id: root

    // Basic dimensions
    property real widthMeters: sceneConfig.roadWidth
    property real lengthMeters: sceneConfig.roadLength
    property real thicknessMeters: sceneConfig.roadThickness

    // Line styling
    property real edgeLineWidth: sceneConfig.edgeLineWidth
    property real centerLineWidth: sceneConfig.centerLineWidth
    property int dashCount: sceneConfig.centerDashCount

    readonly property real planeY: sceneConfig.roadYPosition
    readonly property real lineY: planeY + sceneConfig.lineYOffset

    // Road surface
    Model {
        id: plane
        source: "#Cube"
        position: Qt.vector3d(0, planeY, 0)
        scale: Qt.vector3d(widthMeters, thicknessMeters, lengthMeters)

        materials: PrincipledMaterial {
            baseColor: "#2d2d2d"
            metalness: sceneConfig.roadMetalness
            roughness: sceneConfig.roadRoughness
            specularAmount: sceneConfig.roadSpecular
            emissiveFactor: Qt.vector3d(sceneConfig.roadEmissive, sceneConfig.roadEmissive, sceneConfig.roadEmissive)
        }
    }

    // Left edge line
    Model {
        source: "#Cube"
        position: Qt.vector3d(-widthMeters / 2 + edgeLineWidth / 2, lineY, 0)
        scale: Qt.vector3d(edgeLineWidth, thicknessMeters * 2, lengthMeters)
        materials: PrincipledMaterial {
            baseColor: "#d0d0d0"
            roughness: 0.75
            specularAmount: 0.0
            emissiveFactor: Qt.vector3d(0.5, 0.5, 0.5)
        }
    }

    // Right edge line
    Model {
        source: "#Cube"
        position: Qt.vector3d(widthMeters / 2 - edgeLineWidth / 2, lineY, 0)
        scale: Qt.vector3d(edgeLineWidth, thicknessMeters * 2, lengthMeters)
        materials: PrincipledMaterial {
            baseColor: "#d0d0d0"
            roughness: 0.75
            specularAmount: 0.0
            emissiveFactor: Qt.vector3d(0.5, 0.5, 0.5)
        }
    }

    // Center dashed line
    Repeater3D {
        model: dashCount

        delegate: Model {
            source: "#Cube"

            readonly property real dashSpacing: root.lengthMeters / root.dashCount
            readonly property real dashLength: dashSpacing * sceneConfig.dashLengthRatio

            position: Qt.vector3d(0, lineY, -root.lengthMeters / 2 + (index + 0.5) * dashSpacing)
            scale: Qt.vector3d(root.centerLineWidth, root.thicknessMeters * 2, dashLength)

            materials: PrincipledMaterial {
                baseColor: "#ededed"
                roughness: 0.65
                specularAmount: 0.0
                emissiveFactor: Qt.vector3d(0.6, 0.6, 0.6)
            }
        }
    }
}
