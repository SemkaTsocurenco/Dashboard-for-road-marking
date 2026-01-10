import QtQuick
import QtQuick3D

// Simple car placeholder composed of stacked cubes for visibility
// Real car dimensions: 1.8m wide × 4.5m long × 1.5m tall
// Scaled by scale_factor from scene config
Node {
    // Real car dimensions in meters
    readonly property real carWidthM: 1.8
    readonly property real carLengthM: 4.5
    readonly property real carHeightM: 0.7

    // Apply scale_factor
    readonly property real scaleFactor: sceneConfig.scaleFactor
    readonly property real carWidth: carWidthM * scaleFactor
    readonly property real carLength: carLengthM * scaleFactor
    readonly property real carHeight: carHeightM * scaleFactor




    // Main body (bright)
    Model {
        source: "#Cube"
        position: Qt.vector3d(0, carHeight/2, 0)
        scale: Qt.vector3d(carWidthM, carHeightM , carLengthM )
        materials: PrincipledMaterial {
            baseColor: "#00e0ff"
            metalness: 0.05
            roughness: 0.55
            specularAmount: 0.0
            emissiveFactor: Qt.vector3d(0.7, 0.9, 1.0)
        }
    }
}
