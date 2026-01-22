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
    readonly property real lineY: planeY + 10

    // Apply scale_factor to convert meters to scene units
    readonly property real scaleFactor: sceneConfig.scaleFactor
    readonly property real scaledWidth: widthMeters * scaleFactor
    readonly property real scaledLength: lengthMeters * scaleFactor 
    readonly property real scaledEdgeLineWidth: edgeLineWidth * scaleFactor
    readonly property real scaledCenterLineWidth: centerLineWidth * scaleFactor

    // Road surface - realistic dark asphalt
    Model {
        id: plane
        source: "#Cube"
        position: Qt.vector3d(0, planeY, scaledLength/2 -800)
        scale: Qt.vector3d(widthMeters, thicknessMeters, lengthMeters)

        materials: PrincipledMaterial {
            baseColor: "#1a1d20"
            metalness: 0.0
            roughness: 0.85
            specularAmount: 0.15
            emissiveFactor: Qt.vector3d(0.02, 0.02, 0.03)
        }
    }

    // // Left edge line - bright white with glow
    // Model {
    //     source: "#Cube"
    //     position: Qt.vector3d(-scaledWidth / 2 + scaledEdgeLineWidth / 2, lineY,  scaledLength/2 -800)
    //     scale: Qt.vector3d(edgeLineWidth, thicknessMeters * 2, lengthMeters)
    //     materials: PrincipledMaterial {
    //         baseColor: "#f0f4f8"
    //         roughness: 0.35
    //         specularAmount: 0.2
    //         metalness: 0.0
    //         emissiveFactor: Qt.vector3d(0.85, 0.88, 0.92)
    //     }
    // }

    // // Right edge line - bright white with glow
    // Model {
    //     source: "#Cube"
    //     position: Qt.vector3d(scaledWidth / 2 - scaledEdgeLineWidth / 2, lineY,  scaledLength/2 -800)
    //     scale: Qt.vector3d(edgeLineWidth, thicknessMeters * 2, lengthMeters)
    //     materials: PrincipledMaterial {
    //         baseColor: "#f0f4f8"
    //         roughness: 0.35
    //         specularAmount: 0.2
    //         metalness: 0.0
    //         emissiveFactor: Qt.vector3d(0.85, 0.88, 0.92)
    //     }
    // }

    // // Center dashed line - bright with subtle glow
    // Repeater3D {
    //     model: dashCount

    //     delegate: Model {
    //         required property int index
    //         source: "#Cube"

    //         readonly property real dashSpacing: root.scaledLength / root.dashCount
    //         readonly property real dashLength: dashSpacing * sceneConfig.dashLengthRatio

    //         position: Qt.vector3d(0, lineY, (index + 0.5) * dashSpacing - 800)
    //         scale: Qt.vector3d(root.centerLineWidth, root.thicknessMeters * 2, dashLength)

    //         materials: PrincipledMaterial {
    //             baseColor: "#ffffff"
    //             roughness: 0.3
    //             specularAmount: 0.25
    //             metalness: 0.0
    //             emissiveFactor: Qt.vector3d(0.9, 0.92, 0.95)
    //         }
    //     }
    // }
}
