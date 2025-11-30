import QtQuick
import QtQuick3D

Model {
    id: root

    required property real xMeters
    required property real yMeters
    required property real lengthMeters
    required property real widthMeters
    required property real yawDeg
    required property string className
    required property bool isCrosswalk
    required property bool isArrow
    required property bool isValid
    required property int confidence

    source: "#Cube"
    visible: isValid

    // Keep markings slightly above the plane to avoid z-fighting
    position: Qt.vector3d(xMeters, 0.02, yMeters)

    // Use real-world meters directly
    scale: Qt.vector3d(
        Math.max(widthMeters, 0.05),
        0.03,
        Math.max(lengthMeters, 0.2)
    )

    eulerRotation.y: -yawDeg

    // Reduce opacity if confidence is low
    opacity: Math.max(0.3, Math.min(1.0, confidence / 100.0))

    materials: [
        PrincipledMaterial {
            property color laneColor: {
                if (isCrosswalk) {
                    return "#ffffff"
                } else if (isArrow) {
                    return "#ffd700"
                } else if (className === "Unknown") {
                    return "#c0c0c0"
                } else {
                    return "#e7f0ff"
                }
            }

            baseColor: laneColor
            emissiveFactor: laneColor
            roughness: 0.55
            metalness: 0.0
        }
    ]
}
