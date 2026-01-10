import QtQuick
import QtQuick3D

Node {
    id: root

    // Properties with defaults (set from delegate)
    property real xMeters: 0
    property real yMeters: 0
    property real lengthMeters: 1
    property real widthMeters: 1
    property real yawDeg: 0
    property string className: ""
    property bool isCrosswalk: false
    property bool isArrow: false
    property bool isValid: false
    property int confidence: 0
    property string lineColor: "White"
    property string lineStyle: "Solid"
    property int classId: 0

    visible: isValid

    // Reduce opacity if confidence is low
    property real objectOpacity: Math.max(sceneConfig.markingMinOpacity,
                                          Math.min(sceneConfig.markingMaxOpacity,
                                                   confidence / sceneConfig.markingConfidenceDivisor))

    // Calculate color based on type
    property color objectColor: {
        const lc = lineColor.toLowerCase()
        if (lc === "yellow") return "#ffd700"
        if (lc === "red")    return "#ff5555"
        if (lc === "white")  return "#ffffff"
        if (isCrosswalk)     return "#ffffff"
        if (isArrow)         return "#ffd700"
        return "#ffffff"
    }

    // Common position and rotation
    property vector3d basePosition: Qt.vector3d(
        xMeters * sceneConfig.scaleFactor,
        sceneConfig.markingYPosition,
        yMeters * sceneConfig.scaleFactor
    )

    // Crosswalk: zebra pattern with stripes
    Node {
        id: crosswalkNode
        visible: root.isCrosswalk
        position: root.basePosition
        eulerRotation.y: -root.yawDeg

        property int stripeCount: 8
        property real stripeWidth: root.widthMeters / stripeCount

        Repeater3D {
            model: crosswalkNode.stripeCount

            Model {
                required property int index
                visible: index % 2 === 0  // Every other stripe

                source: "#Cube"

                position: Qt.vector3d(
                    (index - crosswalkNode.stripeCount / 2 + 0.5) * crosswalkNode.stripeWidth,
                    10,
                    0
                )

                scale: Qt.vector3d(
                    crosswalkNode.stripeWidth * 0.9,
                    sceneConfig.markingHeight,
                    root.lengthMeters
                )

                opacity: root.objectOpacity

                materials: PrincipledMaterial {
                    baseColor: root.objectColor
                    emissiveFactor: Qt.vector3d(0.4, 0.4, 0.4)
                    roughness: 0.3
                    metalness: 0.0
                }
            }
        }
    }

    // Arrow: simplified arrow shape
    Node {
        id: arrowNode
        visible: root.isArrow
        position: root.basePosition
        eulerRotation.y: -root.yawDeg

        // Arrow shaft
        Model {
            source: "#Cube"
            position: Qt.vector3d(0, 0, root.lengthMeters * 0.1)
            scale: Qt.vector3d(
                root.widthMeters * 0.2,
                sceneConfig.markingHeight,
                root.lengthMeters * 0.6
            )
            opacity: root.objectOpacity

            materials: PrincipledMaterial {
                baseColor: root.objectColor
                emissiveFactor: Qt.vector3d(
                    root.objectColor.r * 0.5,
                    root.objectColor.g * 0.5,
                    root.objectColor.b * 0.5
                )
                roughness: 0.3
                metalness: 0.0
            }
        }

        // Arrow head (triangular shape using 3 cubes)
        Model {
            source: "#Cube"
            position: Qt.vector3d(0, 0, -root.lengthMeters * 0.3)
            scale: Qt.vector3d(
                root.widthMeters * 0.6,
                sceneConfig.markingHeight,
                root.lengthMeters * 0.15
            )
            opacity: root.objectOpacity

            materials: PrincipledMaterial {
                baseColor: root.objectColor
                emissiveFactor: Qt.vector3d(
                    root.objectColor.r * 0.5,
                    root.objectColor.g * 0.5,
                    root.objectColor.b * 0.5
                )
                roughness: 0.3
                metalness: 0.0
            }
        }

        // Arrow head tip
        Model {
            source: "#Cube"
            position: Qt.vector3d(0, 0, -root.lengthMeters * 0.45)
            scale: Qt.vector3d(
                root.widthMeters * 0.3,
                sceneConfig.markingHeight,
                root.lengthMeters * 0.1
            )
            opacity: root.objectOpacity

            materials: PrincipledMaterial {
                baseColor: root.objectColor
                emissiveFactor: Qt.vector3d(
                    root.objectColor.r * 0.5,
                    root.objectColor.g * 0.5,
                    root.objectColor.b * 0.5
                )
                roughness: 0.3
                metalness: 0.0
            }
        }

        // Left/Right arrow wings (based on classId)
        // ArrowLeft = 11, ArrowRight = 13, ArrowLeftStraight = 14, ArrowRightStraight = 15
        Model {
            visible: root.classId === 11 || root.classId === 14  // Left arrows
            source: "#Cube"
            position: Qt.vector3d(-root.widthMeters * 0.25, 0, 0)
            scale: Qt.vector3d(
                root.widthMeters * 0.3,
                sceneConfig.markingHeight,
                root.lengthMeters * 0.15
            )
            opacity: root.objectOpacity

            materials: PrincipledMaterial {
                baseColor: root.objectColor
                emissiveFactor: Qt.vector3d(
                    root.objectColor.r * 0.5,
                    root.objectColor.g * 0.5,
                    root.objectColor.b * 0.5
                )
                roughness: 0.3
                metalness: 0.0
            }
        }

        Model {
            visible: root.classId === 13 || root.classId === 15  // Right arrows
            source: "#Cube"
            position: Qt.vector3d(root.widthMeters * 0.25, 0, 0)
            scale: Qt.vector3d(
                root.widthMeters * 0.3,
                sceneConfig.markingHeight,
                root.lengthMeters * 0.15
            )
            opacity: root.objectOpacity

            materials: PrincipledMaterial {
                baseColor: root.objectColor
                emissiveFactor: Qt.vector3d(
                    root.objectColor.r * 0.5,
                    root.objectColor.g * 0.5,
                    root.objectColor.b * 0.5
                )
                roughness: 0.3
                metalness: 0.0
            }
        }
    }

    // Other objects: simple flat rectangle
    Model {
        id: genericObject
        visible: !root.isCrosswalk && !root.isArrow
        source: "#Cube"

        position: root.basePosition

        scale: Qt.vector3d(
            root.widthMeters,
            sceneConfig.markingHeight,
            root.lengthMeters
        )

        eulerRotation.y: -root.yawDeg
        opacity: root.objectOpacity

        materials: PrincipledMaterial {
            baseColor: root.objectColor
            emissiveFactor: Qt.vector3d(
                root.objectColor.r * sceneConfig.markingEmissiveFactorScale,
                root.objectColor.g * sceneConfig.markingEmissiveFactorScale,
                root.objectColor.b * sceneConfig.markingEmissiveFactorScale
            )
            roughness: root.lineStyle.toLowerCase() === "dashed" ? sceneConfig.markingRoughnessDashed : sceneConfig.markingRoughnessSolid
            metalness: 0.0
        }
    }
}
