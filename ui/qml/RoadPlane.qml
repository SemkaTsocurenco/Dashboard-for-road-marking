import QtQuick
import QtQuick3D

Node {
    id: root

    // Basic dimensions in meters
    property real widthMeters: sceneConfig.roadWidth
    property real lengthMeters: sceneConfig.roadLength
    property real thicknessMeters: sceneConfig.roadThickness

    // Line styling
    property real edgeLineWidth: sceneConfig.edgeLineWidth
    property real centerLineWidth: sceneConfig.centerLineWidth
    property int dashCount: sceneConfig.centerDashCount

    readonly property real planeY: sceneConfig.roadYPosition
    readonly property real lineY: planeY + sceneConfig.lineYOffset

    // Apply scale_factor to convert meters to scene units
    readonly property real scaleFactor: sceneConfig.scaleFactor
    readonly property real scaledWidth: widthMeters * scaleFactor
    readonly property real scaledLength: lengthMeters * scaleFactor
    readonly property real scaledEdgeLineWidth: edgeLineWidth * scaleFactor
    readonly property real scaledCenterLineWidth: centerLineWidth * scaleFactor


    // Road surface
    // Road starts from car position and extends forward (negative Z direction)
    // Cube primitive goes from 0 to 1, so position at -scaledLength makes it go from Z=0 to Z=-scaledLength
    Model {
        id: plane
        source: "#Cube"
        position: Qt.vector3d(0, planeY, scaledLength/2 - 300)
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
        position: Qt.vector3d(-scaledWidth / 2 + scaledEdgeLineWidth / 2, lineY, scaledLength/2- 300)
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
        position: Qt.vector3d(scaledWidth / 2 - scaledEdgeLineWidth / 2, lineY, scaledLength/2- 300)
        scale: Qt.vector3d(edgeLineWidth, thicknessMeters * 2, lengthMeters)
        materials: PrincipledMaterial {
            baseColor: "#d0d0d0"
            roughness: 0.75
            specularAmount: 0.0
            emissiveFactor: Qt.vector3d(0.5, 0.5, 0.5)
        }
    }


    Repeater3D {
        model: dashCount

        delegate: Model {
            source: "#Cube"

            readonly property real dashSpacing: root.scaledLength / root.dashCount
            readonly property real dashLength: dashSpacing * sceneConfig.dashLengthRatio

            // Start from Z=0 (car position) and go forward (negative Z)
            position: Qt.vector3d(0, lineY, (index + 0.5) * dashSpacing - 300)
            scale: Qt.vector3d(root.centerLineWidth, root.thicknessMeters * 2, 1)

            materials: PrincipledMaterial {
                baseColor: "#ededed"
                roughness: 0.65
                specularAmount: 0.0
                emissiveFactor: Qt.vector3d(0.6, 0.6, 0.6)
            }
        }
    }


}
