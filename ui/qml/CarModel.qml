import QtQuick
import QtQuick3D

// Bus model loaded from GLB file
// Real bus dimensions: 1.8m wide × 4.5m long × 0.7m tall (using current dimensions)
// Scaled by scale_factor from scene config
Node {
    // Real bus dimensions in meters (keeping current dimensions)
    readonly property real carWidthM: 1.8
    readonly property real carLengthM: 4.5
    readonly property real carHeightM: 0.7

    // Apply scale_factor
    readonly property real scaleFactor: sceneConfig.scaleFactor
    readonly property real carWidth: carWidthM * scaleFactor
    readonly property real carLength: carLengthM * scaleFactor
    readonly property real carHeight: carHeightM * scaleFactor

    // Bus model from converted files
    Loader3D {
        source: "qrc:/qml/Bus.qml"
        scale: Qt.vector3d(scaleFactor, scaleFactor, scaleFactor)
    }
}
