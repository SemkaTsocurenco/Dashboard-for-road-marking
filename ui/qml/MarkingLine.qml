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

    readonly property real widthM: Math.max(Number(root.lengthMeters) || 0, sceneConfig.markingMinLength) 
    readonly property real lengthM: Math.max(Number(root.widthMeters) || 0, sceneConfig.markingMinWidth) 

    // Reduce opacity if confidence is low
    property real objectOpacity: Math.max(sceneConfig.markingMinOpacity,
                                          Math.min(sceneConfig.markingMaxOpacity,
                                                   confidence / sceneConfig.markingConfidenceDivisor))

    // Calculate color based on type - vibrant colors with better contrast
    property color objectColor: {
        const lc = lineColor.toLowerCase()
        if (lc === "yellow") return "#ffcc00"
        if (lc === "red")    return "#ff4444"
        if (lc === "white")  return "#f8fafc"
        if (isCrosswalk)     return "#f0f4f8"
        if (isArrow)         return "#ffc107"
        return "#f0f4f8"
    }

    readonly property bool isMotorIcon: root.classId === 0x16
    readonly property bool isBikeIcon: root.classId === 0x17
    readonly property bool isRoadIcon: root.isMotorIcon || root.isBikeIcon

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

        // Zebra crossing: stripes extend along Z axis (road direction), positioned along X axis (across road)
        // widthM = road width (X axis), lengthM = crosswalk depth (Z axis)
        readonly property real stripeThicknessMeters: 0.8
        readonly property real gapThicknessMeters: 0.6
        // Fill entire road width with stripes
        readonly property int stripeCount: Math.max(4, Math.floor(root.lengthM / (stripeThicknessMeters + gapThicknessMeters)))
        readonly property real actualStripeThickness: stripeThicknessMeters

        Component.onCompleted: {
            console.log("Crosswalk created: stripeCount=", stripeCount,
                       "lengthM (Z/depth)=", root.lengthM.toFixed(2),
                       "widthM (X/road width)=", root.widthM.toFixed(2))
        }

        Repeater3D {
            model: crosswalkNode.stripeCount

            Model {
                required property int index
                source: "#Cube"


                // Position stripes evenly across the road (local X)
                position: Qt.vector3d(
                    (index - crosswalkNode.stripeCount / 2 + 0.5) * (crosswalkNode.stripeThicknessMeters + crosswalkNode.gapThicknessMeters) * sceneConfig.scaleFactor,
                    0,
                    0
                )

                scale: Qt.vector3d(
                    crosswalkNode.actualStripeThickness,  // stripe thickness (X axis)
                    sceneConfig.markingHeight,
                    root.lengthM  // stripe length along road (Z axis)
                )

                opacity: root.objectOpacity
                castsShadows: false
                receivesShadows: false

                materials: PrincipledMaterial {
                    baseColor: "#f8fafc"
                    emissiveFactor: Qt.vector3d(0.95, 0.97, 1.0)
                    roughness: 0.25
                    metalness: 0.0
                    specularAmount: 0.3
                }
            }
        }
    }



    // Arrows: render per classId (straight / turn / combined) so they look like actual pavement arrows.
    Node {
        id: arrowNode
        visible: root.isArrow
        position: root.basePosition

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

        // Common material properties - enhanced glow
        property vector3d emissive: Qt.vector3d(
            root.objectColor.r * 0.95,
            root.objectColor.g * 0.85,
            root.objectColor.b * 0.3
        )

        // Helper for consistent material on all arrow parts - brighter with glow
        component ArrowMaterial: PrincipledMaterial {
            baseColor: root.objectColor
            emissiveFactor: arrowNode.emissive
            roughness: 0.2
            metalness: 0.0
            specularAmount: 0.35
        }

        // Straight arrow (points to +Z in local space).
        Node {
            id: straightArrowSingle
            visible: arrowNode.isStraightOnly

            readonly property real tailZ: -arrowNode.totalLength / 2
            readonly property real shaftLength: arrowNode.totalLength * 0.60
            readonly property real headLength: arrowNode.totalLength * 0.40
            readonly property real shaftWidth: Math.max(arrowNode.totalWidth * 0.25, sceneConfig.markingMinWidth * 0.6)
            readonly property real headBaseWidth: arrowNode.totalWidth * 0.75

            // Arrow shaft (ствол стрелки)
            Model {
                source: "#Cube"
                position: Qt.vector3d(0, 10, straightArrowSingle.tailZ + straightArrowSingle.shaftLength / 2)
                scale: Qt.vector3d(straightArrowSingle.shaftWidth, sceneConfig.markingHeight,straightArrowSingle.shaftLength )
                opacity: root.objectOpacity
                castsShadows: false
                receivesShadows: false
                materials: ArrowMaterial { }
            }

            // Arrow head (головка стрелки - конус, повернутый на 90 градусов)
            Model {
                source: "#Cone"
                position: Qt.vector3d(0, 10, straightArrowSingle.tailZ + straightArrowSingle.shaftLength + straightArrowSingle.headLength / 2)
                scale: Qt.vector3d(  straightArrowSingle.headBaseWidth , straightArrowSingle.headLength, 0.05)
                eulerRotation: Qt.vector3d(90, 0, 0)
                opacity: root.objectOpacity
                castsShadows: false
                receivesShadows: false
                materials: ArrowMaterial { }
            }
        }

        // Turn arrow: stem to +Z, then head to -X (left) or +X (right).
        Node {
            id: turnArrowSingle
            visible: arrowNode.isLeftOnly || arrowNode.isRightOnly

            readonly property bool isLeft: arrowNode.isLeftOnly
            readonly property real dir: isLeft ? -1.0 : 1.0

            readonly property real tailZ: -arrowNode.totalLength / 2
            readonly property real stemLength: arrowNode.totalLength * 0.60
            readonly property real bendZ: tailZ + stemLength
            readonly property real stemWidth: Math.max(arrowNode.totalWidth * 0.25, sceneConfig.markingMinWidth * 0.6)

            readonly property real sideShaftLength: arrowNode.totalWidth * 0.25
            readonly property real sideHeadLength: arrowNode.totalWidth * 0.40
            readonly property real sideHeadBaseWidth: stemWidth * 3.0

            // Stem (forward).
            Model {
                source: "#Cube"
                position: Qt.vector3d(0, 10, turnArrowSingle.tailZ + turnArrowSingle.stemLength / 2)
                scale: Qt.vector3d(turnArrowSingle.stemWidth, sceneConfig.markingHeight, turnArrowSingle.stemLength)
                opacity: root.objectOpacity
                castsShadows: false
                receivesShadows: false
                materials: ArrowMaterial { }
            }

            // Side shaft.
            Model {
                source: "#Cube"
                position: Qt.vector3d(turnArrowSingle.dir * (turnArrowSingle.sideShaftLength / 2), 10, turnArrowSingle.bendZ)
                scale: Qt.vector3d(turnArrowSingle.sideShaftLength, sceneConfig.markingHeight, turnArrowSingle.stemWidth)
                opacity: root.objectOpacity
                castsShadows: false
                receivesShadows: false
                materials: ArrowMaterial { }
            }

            // Side arrow head (cone).
            Model {
                source: "#Cone"
                position: Qt.vector3d(
                    turnArrowSingle.dir * (turnArrowSingle.sideShaftLength + turnArrowSingle.sideHeadLength / 2),
                    10,
                    turnArrowSingle.bendZ
                )
                eulerRotation: Qt.vector3d(0, turnArrowSingle.isLeft ? -90 : 90, 0)
                scale: Qt.vector3d(turnArrowSingle.sideHeadBaseWidth, turnArrowSingle.sideHeadLength, 0.05)
                opacity: root.objectOpacity
                castsShadows: false
                receivesShadows: false
                materials: ArrowMaterial { }
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
                readonly property real xOffset: arrowNode.isLeftStraight ? arrowPair.pairOffset : -arrowPair.pairOffset

                readonly property real shaftLength: arrowNode.totalLength * 0.60
                readonly property real headLength: arrowNode.totalLength * 0.40
                readonly property real shaftWidth: Math.max(arrowPair.eachWidth * 0.25, sceneConfig.markingMinWidth * 0.5)
                readonly property real headBaseWidth: arrowPair.eachWidth * 0.75

                Model {
                    source: "#Cube"
                    position: Qt.vector3d(straightInPair.xOffset, 10, arrowPair.tailZ + straightInPair.shaftLength / 2)
                    scale: Qt.vector3d(straightInPair.shaftWidth, sceneConfig.markingHeight, straightInPair.shaftLength)
                    opacity: root.objectOpacity
                    castsShadows: false
                    receivesShadows: false
                    materials: ArrowMaterial { }
                }

                Model {
                    source: "#Cone"
                    position: Qt.vector3d(straightInPair.xOffset, 10, arrowPair.tailZ + straightInPair.shaftLength + straightInPair.headLength / 2)
                    eulerRotation: Qt.vector3d(90, 0, 0)
                    scale: Qt.vector3d(straightInPair.headBaseWidth, straightInPair.headLength, 0.05)
                    opacity: root.objectOpacity
                    castsShadows: false
                    receivesShadows: false
                    materials: ArrowMaterial { }
                }
            }

            // Turn arrow (the other side).
            Node {
                id: turnInPair
                readonly property bool isLeft: arrowNode.isLeftStraight
                readonly property real dir: isLeft ? -1.0 : 1.0
                readonly property real xOffset: isLeft ? -arrowPair.pairOffset : arrowPair.pairOffset

                readonly property real stemLength: arrowNode.totalLength * 0.60
                readonly property real bendZ: arrowPair.tailZ + stemLength
                readonly property real stemWidth: Math.max(arrowPair.eachWidth * 0.25, sceneConfig.markingMinWidth * 0.5)

                readonly property real sideShaftLength: arrowPair.eachWidth * 0.25
                readonly property real sideHeadLength: arrowPair.eachWidth * 0.40
                readonly property real sideHeadBaseWidth: stemWidth * 3.0

                Model {
                    source: "#Cube"
                    position: Qt.vector3d(turnInPair.xOffset, 10, arrowPair.tailZ + turnInPair.stemLength / 2)
                    scale: Qt.vector3d(turnInPair.stemWidth, sceneConfig.markingHeight, turnInPair.stemLength)
                    opacity: root.objectOpacity
                    castsShadows: false
                    receivesShadows: false
                    materials: ArrowMaterial { }
                }

                Model {
                    source: "#Cube"
                    position: Qt.vector3d(turnInPair.xOffset + turnInPair.dir * (turnInPair.sideShaftLength / 2), 10, turnInPair.bendZ)
                    scale: Qt.vector3d(turnInPair.sideShaftLength, sceneConfig.markingHeight, turnInPair.stemWidth)
                    opacity: root.objectOpacity
                    castsShadows: false
                    receivesShadows: false
                    materials: ArrowMaterial { }
                }

                Model {
                    source: "#Cone"
                    position: Qt.vector3d(
                        turnInPair.xOffset + turnInPair.dir * (turnInPair.sideShaftLength + turnInPair.sideHeadLength / 2),
                        10,
                        turnInPair.bendZ
                    )
                    eulerRotation: Qt.vector3d(0, turnInPair.isLeft ? -90 : 90, 0)
                    scale: Qt.vector3d(turnInPair.sideHeadBaseWidth, turnInPair.sideHeadLength, 0.05)
                    opacity: root.objectOpacity
                    castsShadows: false
                    receivesShadows: false
                    materials: ArrowMaterial { }
                }
            }
        }
    }

    // Icons: simplified top-down silhouettes (bike/motor) instead of generic rectangles.
    // Node {
    //     id: iconNode
    //     visible: root.isRoadIcon
    //     position: root.basePosition
    //     eulerRotation.y: -root.yawDeg

    //     readonly property real iconLength: root.lengthM
    //     readonly property real iconWidth: root.widthM

    //     readonly property real wheelRadius: Math.max(sceneConfig.markingMinWidth * 0.6, Math.min(iconWidth, iconLength) * 0.18)
    //     readonly property real stroke: Math.max(sceneConfig.markingMinWidth * 0.25, wheelRadius * 0.35)

    //     readonly property real zWheelFront: iconLength * 0.25
    //     readonly property real zWheelRear: -iconLength * 0.25

    //     PrincipledMaterial {
    //         id: iconMaterial
    //         baseColor: root.objectColor
    //         emissiveFactor: Qt.vector3d(0.55, 0.55, 0.55)
    //         roughness: 0.55
    //         metalness: 0.0
    //         specularAmount: 0.0
    //     }

    //     // Wheels
    //     Model {
    //         source: "#Cylinder"
    //         position: Qt.vector3d(0, 0, iconNode.zWheelRear)
    //         scale: Qt.vector3d(iconNode.wheelRadius, sceneConfig.markingHeight, iconNode.wheelRadius)
    //         opacity: root.objectOpacity
    //         castsShadows: false
    //         receivesShadows: false
    //         materials: [ iconMaterial ]
    //     }

    //     Model {
    //         source: "#Cylinder"
    //         position: Qt.vector3d(0, 0, iconNode.zWheelFront)
    //         scale: Qt.vector3d(iconNode.wheelRadius, sceneConfig.markingHeight, iconNode.wheelRadius)
    //         opacity: root.objectOpacity
    //         castsShadows: false
    //         receivesShadows: false
    //         materials: [ iconMaterial ]
    //     }

    //     // Bike frame (simple line work)
    //     Node {
    //         visible: root.isBikeIcon

    //         Model {
    //             source: "#Cube"
    //             position: Qt.vector3d(0, 0, 0)
    //             scale: Qt.vector3d(iconNode.stroke, sceneConfig.markingHeight, iconNode.iconLength * 0.55)
    //             opacity: root.objectOpacity
    //             castsShadows: false
    //             receivesShadows: false
    //             materials: [ iconMaterial ]
    //         }

    //         Model {
    //             source: "#Cube"
    //             position: Qt.vector3d(iconNode.iconWidth * 0.18, 0, iconNode.zWheelFront * 0.55)
    //             scale: Qt.vector3d(iconNode.iconWidth * 0.36, sceneConfig.markingHeight, iconNode.stroke)
    //             opacity: root.objectOpacity
    //             castsShadows: false
    //             receivesShadows: false
    //             materials: [ iconMaterial ]
    //         }
    //     }

    //     // Motor silhouette (wider body block)
    //     Node {
    //         visible: root.isMotorIcon

    //         Model {
    //             source: "#Cube"
    //             position: Qt.vector3d(0, 0, 0)
    //             scale: Qt.vector3d(iconNode.iconWidth * 0.65, sceneConfig.markingHeight, iconNode.iconLength * 0.35)
    //             opacity: root.objectOpacity
    //             castsShadows: false
    //             receivesShadows: false
    //             materials: [ iconMaterial ]
    //         }

    //         Model {
    //             source: "#Cube"
    //             position: Qt.vector3d(iconNode.iconWidth * 0.18, 0, iconNode.zWheelFront * 0.45)
    //             scale: Qt.vector3d(iconNode.iconWidth * 0.35, sceneConfig.markingHeight, iconNode.stroke)
    //             opacity: root.objectOpacity
    //             castsShadows: false
    //             receivesShadows: false
    //             materials: [ iconMaterial ]
    //         }
    //     }
    // }

    // Other objects: generic marking line/rectangle (style-aware).
    Node {
        id: genericNode
        visible: !root.isCrosswalk && !root.isArrow && !root.isRoadIcon
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
            Math.min(1.0, root.objectColor.r * 0.9),
            Math.min(1.0, root.objectColor.g * 0.9),
            Math.min(1.0, root.objectColor.b * 0.9)
        )

        // Solid style - enhanced visibility
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
                roughness: 0.25
                metalness: 0.0
                specularAmount: 0.3
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
                    roughness: 0.25
                    metalness: 0.0
                    specularAmount: 0.3
                }
            }
        }

        // Double style - enhanced glow
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
                roughness: 0.25
                metalness: 0.0
                specularAmount: 0.3
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
                roughness: 0.25
                metalness: 0.0
                specularAmount: 0.3
            }
        }
    }
}
