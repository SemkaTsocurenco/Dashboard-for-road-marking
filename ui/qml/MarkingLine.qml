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
    }

    source: "#Cube"
    visible: isValid


    position: Qt.vector3d(
        xMeters * sceneConfig.scaleFactor,
        sceneConfig.markingYPosition,
        yMeters * sceneConfig.scaleFactor
    )

    // Scale uses meters directly (scaleFactor applied in position only)
    scale: Qt.vector3d(
        widthMeters, 
        sceneConfig.markingHeight,
        lengthMeters
    )

    eulerRotation.y: -yawDeg

    // Reduce opacity if confidence is low
    opacity: Math.max(sceneConfig.markingMinOpacity, Math.min(sceneConfig.markingMaxOpacity, confidence / sceneConfig.markingConfidenceDivisor))

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
            emissiveFactor: Qt.vector3d(laneColor.r * sceneConfig.markingEmissiveFactorScale, laneColor.g * sceneConfig.markingEmissiveFactorScale, laneColor.b * sceneConfig.markingEmissiveFactorScale)
            roughness: lineStyle.toLowerCase() === "dashed" ? sceneConfig.markingRoughnessDashed : sceneConfig.markingRoughnessSolid
            metalness: 0.0
        }
    ]
}
