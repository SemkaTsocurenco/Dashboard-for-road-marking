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
    required property string lineColor
    required property string lineStyle

    Component.onCompleted: {
        console.log("MarkingLine created: x=", xMeters, "y=", yMeters, "length=", lengthMeters,
                    "width=", widthMeters, "valid=", isValid, "color=", lineColor,
                    "position:", position, "scale:", scale, "visible:", visible)
    }

    source: "#Cube"
    visible: isValid

    // Keep markings slightly above the plane to avoid z-fighting
    // Invert Y coordinate to match camera view direction (camera looks toward -Z)
    position: Qt.vector3d(xMeters, 0.02, -yMeters)

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
                const lc = lineColor.toLowerCase()
                if (lc === "yellow") return "#ffd700"
                if (lc === "red")    return "#ff5555"
                if (lc === "white")  return "#ffffff"
                if (isCrosswalk)     return "#ffffff"
                if (isArrow)         return "#ffd700"
                // Default: bright white for unknown/unspecified
                return "#ffffff"
            }

            baseColor: laneColor
            emissiveFactor: Qt.vector3d(laneColor.r * 0.8, laneColor.g * 0.8, laneColor.b * 0.8)
            roughness: lineStyle.toLowerCase() === "dashed" ? 0.8 : 0.55
            metalness: 0.0
        }
    ]
}
