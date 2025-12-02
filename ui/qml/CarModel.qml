import QtQuick
import QtQuick3D

// Simple car placeholder composed of stacked cubes for visibility
// Real car dimensions: 1.8m wide × 4.5m long × 1.5m tall
// Scaled by scale_factor from scene config
Node {
    // Real car dimensions in meters
    readonly property real carWidthM: 1.8
    readonly property real carLengthM: 4.5
    readonly property real carHeightM: 1.5

    // Apply scale_factor
    readonly property real scaleFactor: sceneConfig.scaleFactor
    readonly property real carWidth: carWidthM * scaleFactor
    readonly property real carLength: carLengthM * scaleFactor
    readonly property real carHeight: carHeightM * scaleFactor

    // Shadow footprint to outline the car
    Model {
        source: "#Cube"
        position: Qt.vector3d(0, carHeight * 0.05, 0)
        scale: Qt.vector3d(carWidth * 1.07, carHeight * 0.1, carLength * 1.04)
        materials: PrincipledMaterial {
            baseColor: "#050505"
            metalness: 0.0
            roughness: 1.0
            specularAmount: 0.0
            emissiveFactor: Qt.vector3d(0.02, 0.02, 0.02)
        }
    }

    // Base body (dark outline)
    Model {
        source: "#Cube"
        position: Qt.vector3d(0, carHeight * 0.25, 0)
        scale: Qt.vector3d(carWidth * 1.0, carHeight * 0.3, carLength * 1.0)
        materials: PrincipledMaterial {
            baseColor: "#0b1620"
            metalness: 0.0
            roughness: 0.9
            specularAmount: 0.0
            emissiveFactor: Qt.vector3d(0.15, 0.15, 0.2)
        }
    }

    // Main body (bright)
    Model {
        source: "#Cube"
        position: Qt.vector3d(0, carHeight * 0.55, 0)
        scale: Qt.vector3d(carWidth * 0.95, carHeight * 0.55, carLength * 0.96)
        materials: PrincipledMaterial {
            baseColor: "#00e0ff"
            metalness: 0.05
            roughness: 0.55
            specularAmount: 0.0
            emissiveFactor: Qt.vector3d(0.7, 0.9, 1.0)
        }
    }

    // Roof highlight
    Model {
        source: "#Cube"
        position: Qt.vector3d(0, carHeight * 0.95, 0)
        scale: Qt.vector3d(carWidth * 0.67, carHeight * 0.3, carLength * 0.56)
        materials: PrincipledMaterial {
            baseColor: "#8cd7ff"
            metalness: 0.0
            roughness: 0.7
            specularAmount: 0.0
            emissiveFactor: Qt.vector3d(0.45, 0.45, 0.55)
        }
    }
}
