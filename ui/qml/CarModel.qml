import QtQuick
import QtQuick3D

// Simple car placeholder composed of stacked cubes for visibility
Node {

    // Shadow footprint to outline the car
    Model {
        source: "#Cube"
        position: Qt.vector3d(0, 0.05, 0)
        scale: Qt.vector3d(2.4, 0.1, 5.2)
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
        position: Qt.vector3d(0, 0.25, 0)
        scale: Qt.vector3d(2.0, 0.45, 4.8)
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
        position: Qt.vector3d(0, 0.55, 0)
        scale: Qt.vector3d(1.8, 0.55, 4.5)
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
        position: Qt.vector3d(0, 0.95, 0)
        scale: Qt.vector3d(1.2, 0.3, 2.5)
        materials: PrincipledMaterial {
            baseColor: "#8cd7ff"
            metalness: 0.0
            roughness: 0.7
            specularAmount: 0.0
            emissiveFactor: Qt.vector3d(0.45, 0.45, 0.55)
        }
    }
}
