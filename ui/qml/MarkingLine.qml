import QtQuick
import QtQuick3D

Node {
    id: root

    // Required properties automatically bound to model roles by Repeater3D
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
    required property int classId

    visible: isValid

    Component.onCompleted: {
        console.log("MarkingLine created:", className,
                    "pos:", xMeters.toFixed(2), yMeters.toFixed(2),
                    "size:", lengthMeters.toFixed(2), "x", widthMeters.toFixed(2),
                    "yaw:", yawDeg.toFixed(1),
                    "isValid:", isValid,
                    "isCrosswalk:", isCrosswalk,
                    "isArrow:", isArrow)
    }

    readonly property real lengthM: Math.max(Number(root.lengthMeters) || 0, sceneConfig.markingMinLength)
    readonly property real widthM: Math.max(Number(root.widthMeters) || 0, sceneConfig.markingMinWidth)

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
        -xMeters * sceneConfig.scaleFactor,  // Negated to match fitted lines coordinate system
        sceneConfig.markingYPosition,
        yMeters * sceneConfig.scaleFactor
    )

    // Crosswalk: zebra pattern with white stripes
    Node {
        id: crosswalkNode
        visible: root.isCrosswalk
        position: root.basePosition
        eulerRotation.y: -root.yawDeg

        // Zebra crossing: stripes repeat along the travel direction (local Z) and span the full width (local X).
        // Typical crosswalk: ~40-50cm stripe thickness along the road direction.
        readonly property real stripeThicknessMeters: 0.5
        readonly property int stripeCount: Math.max(4, Math.floor(root.lengthM / (stripeThicknessMeters * 2)))
        readonly property real actualStripeThickness: root.lengthM / (stripeCount * 2) // white + gap

        Repeater3D {
            model: crosswalkNode.stripeCount

            Model {
                required property int index
                source: "#Cube"

                // Position stripes evenly along length (local Z)
                position: Qt.vector3d(
                    0,
                    0,
                    (index - crosswalkNode.stripeCount / 2 + 0.5) * crosswalkNode.actualStripeThickness * 2
                )

                scale: Qt.vector3d(
                    root.widthM,  // span full crosswalk width
                    sceneConfig.markingHeight,
                    crosswalkNode.actualStripeThickness * 0.92 // slight gap between stripes
                )

                opacity: root.objectOpacity
                castsShadows: false
                receivesShadows: false

                materials: PrincipledMaterial {
                    baseColor: "#ffffff"  // Always white for crosswalk
                    emissiveFactor: Qt.vector3d(0.6, 0.6, 0.6)  // Brighter for visibility
                    roughness: 0.4
                    metalness: 0.0
                    specularAmount: 0.0
                }
            }
        }
    }

    // Arrows: render per classId (straight / turn / combined) so they look like actual pavement arrows.
    Node {
        id: arrowNode
        visible: root.isArrow
        position: root.basePosition
        eulerRotation.y: -root.yawDeg

        readonly property int arrowLeftId: 0x0B
        readonly property int arrowStraightId: 0x0C
        readonly property int arrowRightId: 0x0D
        readonly property int arrowLeftStraightId: 0x0E
        readonly property int arrowRightStraightId: 0x0F

        readonly property bool isLeftOnly: root.classId === arrowLeftId
        readonly property bool isStraightOnly: root.classId === arrowStraightId
        readonly property bool isRightOnly: root.classId === arrowRightId
        readonly property bool isLeftStraight: root.classId === arrowLeftStraightId
        readonly property bool isRightStraight: root.classId === arrowRightStraightId

        readonly property real totalLength: root.lengthM
        readonly property real totalWidth: root.widthM

        // Common material properties
        property vector3d emissive: Qt.vector3d(
            root.objectColor.r * 0.7,
            root.objectColor.g * 0.7,
            root.objectColor.b * 0.7
        )

        // Helper for consistent material on all arrow parts.
        component ArrowMaterial: PrincipledMaterial {
            baseColor: root.objectColor
            emissiveFactor: arrowNode.emissive
            roughness: 0.35
            metalness: 0.0
            specularAmount: 0.0
        }

        // Straight arrow (points to +Z in local space).
        Node {
            id: straightArrowSingle
            visible: arrowNode.isStraightOnly

            readonly property real tailZ: -arrowNode.totalLength / 2
            readonly property real shaftLength: arrowNode.totalLength * 0.62
            readonly property real headLength: arrowNode.totalLength - shaftLength
            readonly property real shaftWidth: Math.max(arrowNode.totalWidth * 0.22, sceneConfig.markingMinWidth * 0.6)
            readonly property real headBaseWidth: arrowNode.totalWidth * 0.92
            readonly property real headTipWidth: Math.max(arrowNode.totalWidth * 0.12, sceneConfig.markingMinWidth * 0.6)
            readonly property int headSegments: Math.max(8, Math.min(18, Math.floor(arrowNode.totalLength * 6)))

            Model {
                source: "#Cube"
                position: Qt.vector3d(0, 0, straightArrowSingle.tailZ + straightArrowSingle.shaftLength / 2)
                scale: Qt.vector3d(straightArrowSingle.shaftWidth, sceneConfig.markingHeight, straightArrowSingle.shaftLength)
                opacity: root.objectOpacity
                castsShadows: false
                receivesShadows: false
                materials: ArrowMaterial { }
            }

            Repeater3D {
                model: straightArrowSingle.headSegments

                delegate: Model {
                    required property int index
                    source: "#Cube"

                    readonly property real segLen: straightArrowSingle.headLength / straightArrowSingle.headSegments
                    readonly property real t: (index + 0.5) / straightArrowSingle.headSegments
                    readonly property real widthAtT: straightArrowSingle.headBaseWidth * (1.0 - t) + straightArrowSingle.headTipWidth * t

                    position: Qt.vector3d(
                        0,
                        0,
                        straightArrowSingle.tailZ + straightArrowSingle.shaftLength + (index + 0.5) * segLen
                    )

                    scale: Qt.vector3d(widthAtT, sceneConfig.markingHeight, segLen * 0.96)
                    opacity: root.objectOpacity
                    castsShadows: false
                    receivesShadows: false
                    materials: ArrowMaterial { }
                }
            }
        }

        // Turn arrow: stem to +Z, then head to -X (left) or +X (right).
        Node {
            id: turnArrowSingle
            visible: arrowNode.isLeftOnly || arrowNode.isRightOnly

            readonly property bool isLeft: arrowNode.isLeftOnly
            readonly property real dir: isLeft ? -1.0 : 1.0

            readonly property real tailZ: -arrowNode.totalLength / 2
            readonly property real stemLength: arrowNode.totalLength * 0.58
            readonly property real bendZ: tailZ + stemLength
            readonly property real stemWidth: Math.max(arrowNode.totalWidth * 0.22, sceneConfig.markingMinWidth * 0.6)

            // Side branch occupies half of available width from center to edge.
            readonly property real sideShaftLength: arrowNode.totalWidth * 0.25
            readonly property real sideHeadLength: arrowNode.totalWidth * 0.25
            readonly property real sideHeadBaseWidth: stemWidth * 2.2
            readonly property real sideHeadTipWidth: stemWidth * 0.55
            readonly property int sideHeadSegments: Math.max(6, Math.min(14, Math.floor(arrowNode.totalWidth * 5)))

            // Stem (forward).
            Model {
                source: "#Cube"
                position: Qt.vector3d(0, 0, turnArrowSingle.tailZ + turnArrowSingle.stemLength / 2)
                scale: Qt.vector3d(turnArrowSingle.stemWidth, sceneConfig.markingHeight, turnArrowSingle.stemLength)
                opacity: root.objectOpacity
                castsShadows: false
                receivesShadows: false
                materials: ArrowMaterial { }
            }

            // Side shaft.
            Model {
                source: "#Cube"
                position: Qt.vector3d(turnArrowSingle.dir * (turnArrowSingle.sideShaftLength / 2), 0, turnArrowSingle.bendZ)
                eulerRotation: Qt.vector3d(0, turnArrowSingle.isLeft ? -90 : 90, 0)
                scale: Qt.vector3d(turnArrowSingle.stemWidth, sceneConfig.markingHeight, turnArrowSingle.sideShaftLength)
                opacity: root.objectOpacity
                castsShadows: false
                receivesShadows: false
                materials: ArrowMaterial { }
            }

            // Side arrow head (triangular, segmented).
            Repeater3D {
                model: turnArrowSingle.sideHeadSegments

                delegate: Model {
                    required property int index
                    source: "#Cube"

                    readonly property real segLen: turnArrowSingle.sideHeadLength / turnArrowSingle.sideHeadSegments
                    readonly property real t: (index + 0.5) / turnArrowSingle.sideHeadSegments
                    readonly property real widthAtT: turnArrowSingle.sideHeadBaseWidth * (1.0 - t) + turnArrowSingle.sideHeadTipWidth * t

                    position: Qt.vector3d(
                        turnArrowSingle.dir * (turnArrowSingle.sideShaftLength + (index + 0.5) * segLen),
                        0,
                        turnArrowSingle.bendZ
                    )
                    eulerRotation: Qt.vector3d(0, turnArrowSingle.isLeft ? -90 : 90, 0)

                    scale: Qt.vector3d(widthAtT, sceneConfig.markingHeight, segLen * 0.96)
                    opacity: root.objectOpacity
                    castsShadows: false
                    receivesShadows: false
                    materials: ArrowMaterial { }
                }
            }
        }

        // Combined arrows: two separate symbols side-by-side.
        Node {
            id: arrowPair
            visible: arrowNode.isLeftStraight || arrowNode.isRightStraight

            readonly property real pairOffset: arrowNode.totalWidth * 0.22
            readonly property real eachWidth: arrowNode.totalWidth * 0.48
            readonly property real tailZ: -arrowNode.totalLength / 2

            // Straight arrow (one side).
            Node {
                id: straightInPair
                readonly property real xOffset: arrowNode.isLeftStraight ? pairOffset : -pairOffset

                readonly property real shaftLength: arrowNode.totalLength * 0.62
                readonly property real headLength: arrowNode.totalLength - shaftLength
                readonly property real shaftWidth: Math.max(arrowPair.eachWidth * 0.22, sceneConfig.markingMinWidth * 0.5)
                readonly property real headBaseWidth: arrowPair.eachWidth * 0.92
                readonly property real headTipWidth: Math.max(arrowPair.eachWidth * 0.12, sceneConfig.markingMinWidth * 0.5)
                readonly property int headSegments: Math.max(8, Math.min(18, Math.floor(arrowNode.totalLength * 6)))

                Model {
                    source: "#Cube"
                    position: Qt.vector3d(straightInPair.xOffset, 0, arrowPair.tailZ + straightInPair.shaftLength / 2)
                    scale: Qt.vector3d(straightInPair.shaftWidth, sceneConfig.markingHeight, straightInPair.shaftLength)
                    opacity: root.objectOpacity
                    castsShadows: false
                    receivesShadows: false
                    materials: ArrowMaterial { }
                }

                Repeater3D {
                    model: straightInPair.headSegments

                    delegate: Model {
                        required property int index
                        source: "#Cube"

                        readonly property real segLen: straightInPair.headLength / straightInPair.headSegments
                        readonly property real t: (index + 0.5) / straightInPair.headSegments
                        readonly property real widthAtT: straightInPair.headBaseWidth * (1.0 - t) + straightInPair.headTipWidth * t

                        position: Qt.vector3d(
                            straightInPair.xOffset,
                            0,
                            arrowPair.tailZ + straightInPair.shaftLength + (index + 0.5) * segLen
                        )

                        scale: Qt.vector3d(widthAtT, sceneConfig.markingHeight, segLen * 0.96)
                        opacity: root.objectOpacity
                        castsShadows: false
                        receivesShadows: false
                        materials: ArrowMaterial { }
                    }
                }
            }

            // Turn arrow (the other side).
            Node {
                id: turnInPair
                readonly property bool isLeft: arrowNode.isLeftStraight
                readonly property real dir: isLeft ? -1.0 : 1.0
                readonly property real xOffset: isLeft ? -pairOffset : pairOffset

                readonly property real stemLength: arrowNode.totalLength * 0.58
                readonly property real bendZ: arrowPair.tailZ + stemLength
                readonly property real stemWidth: Math.max(arrowPair.eachWidth * 0.22, sceneConfig.markingMinWidth * 0.5)

                readonly property real sideShaftLength: arrowPair.eachWidth * 0.25
                readonly property real sideHeadLength: arrowPair.eachWidth * 0.25
                readonly property real sideHeadBaseWidth: stemWidth * 2.2
                readonly property real sideHeadTipWidth: stemWidth * 0.55
                readonly property int sideHeadSegments: Math.max(6, Math.min(14, Math.floor(arrowPair.eachWidth * 5)))

                Model {
                    source: "#Cube"
                    position: Qt.vector3d(turnInPair.xOffset, 0, arrowPair.tailZ + turnInPair.stemLength / 2)
                    scale: Qt.vector3d(turnInPair.stemWidth, sceneConfig.markingHeight, turnInPair.stemLength)
                    opacity: root.objectOpacity
                    castsShadows: false
                    receivesShadows: false
                    materials: ArrowMaterial { }
                }

                Model {
                    source: "#Cube"
                    position: Qt.vector3d(turnInPair.xOffset + turnInPair.dir * (turnInPair.sideShaftLength / 2), 0, turnInPair.bendZ)
                    eulerRotation: Qt.vector3d(0, turnInPair.isLeft ? -90 : 90, 0)
                    scale: Qt.vector3d(turnInPair.stemWidth, sceneConfig.markingHeight, turnInPair.sideShaftLength)
                    opacity: root.objectOpacity
                    castsShadows: false
                    receivesShadows: false
                    materials: ArrowMaterial { }
                }

                Repeater3D {
                    model: turnInPair.sideHeadSegments

                    delegate: Model {
                        required property int index
                        source: "#Cube"

                        readonly property real segLen: turnInPair.sideHeadLength / turnInPair.sideHeadSegments
                        readonly property real t: (index + 0.5) / turnInPair.sideHeadSegments
                        readonly property real widthAtT: turnInPair.sideHeadBaseWidth * (1.0 - t) + turnInPair.sideHeadTipWidth * t

                        position: Qt.vector3d(
                            turnInPair.xOffset + turnInPair.dir * (turnInPair.sideShaftLength + (index + 0.5) * segLen),
                            0,
                            turnInPair.bendZ
                        )
                        eulerRotation: Qt.vector3d(0, turnInPair.isLeft ? -90 : 90, 0)

                        scale: Qt.vector3d(widthAtT, sceneConfig.markingHeight, segLen * 0.96)
                        opacity: root.objectOpacity
                        castsShadows: false
                        receivesShadows: false
                        materials: ArrowMaterial { }
                    }
                }
            }
        }
    }

    // Other objects: generic marking line/rectangle (style-aware).
    Node {
        id: genericNode
        visible: !root.isCrosswalk && !root.isArrow
        position: root.basePosition
        eulerRotation.y: -root.yawDeg
        opacity: root.objectOpacity

        readonly property string styleLower: String(root.lineStyle || "").toLowerCase()
        readonly property bool isDashedStyle: styleLower === "dashed"
        readonly property bool isDoubleStyle: styleLower === "double"
        readonly property bool isSolidStyle: !isDashedStyle && !isDoubleStyle

        readonly property real markingRoughness: isDashedStyle ? sceneConfig.markingRoughnessDashed : sceneConfig.markingRoughnessSolid

        // Dashed parameters (in meters, local Z axis is length direction)
        readonly property real dashPeriodMeters: 1.0
        readonly property int dashCount: Math.max(1, Math.floor(root.lengthM / dashPeriodMeters))
        readonly property real dashLengthMeters: dashPeriodMeters * sceneConfig.dashLengthRatio

        // Double parameters (fit into provided widthMeters)
        readonly property real doubleLineGap: root.widthM * 0.2
        readonly property real doubleLineWidth: root.widthM * 0.4
        readonly property real doubleLineOffset: (doubleLineGap + doubleLineWidth) * 0.5

        readonly property vector3d emissive: Qt.vector3d(
            root.objectColor.r * sceneConfig.markingEmissiveFactorScale,
            root.objectColor.g * sceneConfig.markingEmissiveFactorScale,
            root.objectColor.b * sceneConfig.markingEmissiveFactorScale
        )

        // Solid style
        Model {
            visible: genericNode.isSolidStyle
            source: "#Cube"
            scale: Qt.vector3d(
                root.widthM,
                sceneConfig.markingHeight,
                root.lengthM
            )
            castsShadows: false
            receivesShadows: false

            materials: PrincipledMaterial {
                baseColor: root.objectColor
                emissiveFactor: genericNode.emissive
                roughness: genericNode.markingRoughness
                metalness: 0.0
                specularAmount: 0.0
            }
        }

        // Dashed style
        Repeater3D {
            model: genericNode.isDashedStyle ? genericNode.dashCount : 0

            delegate: Model {
                required property int index
                source: "#Cube"

                readonly property real period: genericNode.dashPeriodMeters
                readonly property real z0: -root.lengthM / 2 + (index + 0.5) * period

                position: Qt.vector3d(0, 0, z0)
                scale: Qt.vector3d(
                    root.widthM,
                    sceneConfig.markingHeight,
                    genericNode.dashLengthMeters
                )
                castsShadows: false
                receivesShadows: false

                materials: PrincipledMaterial {
                    baseColor: root.objectColor
                    emissiveFactor: genericNode.emissive
                    roughness: genericNode.markingRoughness
                    metalness: 0.0
                    specularAmount: 0.0
                }
            }
        }

        // Double style
        Model {
            visible: genericNode.isDoubleStyle
            source: "#Cube"
            position: Qt.vector3d(genericNode.doubleLineOffset, 0, 0)
            scale: Qt.vector3d(
                genericNode.doubleLineWidth,
                sceneConfig.markingHeight,
                root.lengthM
            )
            castsShadows: false
            receivesShadows: false

            materials: PrincipledMaterial {
                baseColor: root.objectColor
                emissiveFactor: genericNode.emissive
                roughness: genericNode.markingRoughness
                metalness: 0.0
                specularAmount: 0.0
            }
        }

        Model {
            visible: genericNode.isDoubleStyle
            source: "#Cube"
            position: Qt.vector3d(-genericNode.doubleLineOffset, 0, 0)
            scale: Qt.vector3d(
                genericNode.doubleLineWidth,
                sceneConfig.markingHeight,
                root.lengthM
            )
            castsShadows: false
            receivesShadows: false

            materials: PrincipledMaterial {
                baseColor: root.objectColor
                emissiveFactor: genericNode.emissive
                roughness: genericNode.markingRoughness
                metalness: 0.0
                specularAmount: 0.0
            }
        }
    }
}
