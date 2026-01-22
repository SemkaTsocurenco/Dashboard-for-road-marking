pragma ComponentBehavior: Bound

import QtQuick
import QtQuick3D

Node {
    id: carScene
    // Expose camera to parent (Dashboard.qml)
    property alias camera: camera

    // Camera placement tuned for a balanced isometric view
    property real cameraHeight: sceneConfig.cameraHeight
    property real cameraDistance: sceneConfig.cameraDistance

    readonly property real lanePointRadius: 0.95
    readonly property real lanePointHeight: 0.05
    readonly property real laneLineWidth: sceneConfig.edgeLineWidth
    readonly property real laneLineHeight: sceneConfig.roadThickness * 2
    readonly property real laneLineY: lanePointHeight * 0.5
    readonly property int fittedLineCurveSegments: 20
    readonly property real fittedLineDoubleOffsetMeters: laneLineWidth * 1.5

    PerspectiveCamera {
        id: camera
        position: Qt.vector3d(1000, cameraHeight, cameraDistance) // look toward -Z
        eulerRotation: Qt.vector3d(sceneConfig.cameraPitchAngle, 153, 0) // tilt downward toward the road
        fieldOfView: 60
        clipNear: sceneConfig.cameraClipNear
        clipFar: sceneConfig.cameraClipFar
    }

    // Main key light - warm sunlight from above-front
    DirectionalLight {
        eulerRotation: Qt.vector3d(-50, 25, 0)
        brightness: sceneConfig.mainLightBrightness * 1.2
        castsShadow: true
        shadowMapQuality: Light.ShadowMapQualityHigh
        shadowBias: 0.5
        shadowFactor: 35
        color: "#fff5e6"
    }

    // Fill light - cool blue from the side for contrast
    DirectionalLight {
        eulerRotation: Qt.vector3d(30, -45, 0)
        brightness: sceneConfig.secondaryLightBrightness * 0.9
        castsShadow: false
        color: "#b8d4e8"
    }

    // Rim/back light - cyan accent for edge definition
    DirectionalLight {
        eulerRotation: Qt.vector3d(-10, -160, 0)
        brightness: sceneConfig.tertiaryLightBrightness * 1.1
        castsShadow: false
        color: "#4de8e8"
    }

    // Ambient fill from below - subtle road reflection
    DirectionalLight {
        eulerRotation: Qt.vector3d(85, 0, 0)
        brightness: 0.15
        castsShadow: false
        color: "#3a4550"
    }

    // Local spotlight on car - dramatic highlight
    SpotLight {
        position: Qt.vector3d(0, sceneConfig.pointLightYPosition + 100, -200)
        eulerRotation: Qt.vector3d(-45, 0, 0)
        brightness: sceneConfig.pointLightBrightness * 1.5
        color: "#ffffff"
        coneAngle: 55
        innerConeAngle: 35
        quadraticFade: sceneConfig.pointLightQuadraticFade
        linearFade: sceneConfig.pointLightLinearFade * 0.5
        constantFade: sceneConfig.pointLightConstantFade
    }

    // Accent point light - cyan glow for futuristic feel
    PointLight {
        position: Qt.vector3d(150, 50, 100)
        brightness: 0.4
        color: "#39b9c6"
        quadraticFade: 0.0001
        linearFade: 0.001
        constantFade: 1.0
    }

    RoadPlane {
        widthMeters: sceneConfig.roadWidth
        lengthMeters: sceneConfig.roadLength
    }

    CarModel {
        position: Qt.vector3d(0, 0, 0)
    }

    // Marking Objects (crosswalks, arrows, etc.)
    Repeater3D {
        id: markingRepeater
        model: markingModel

        Component.onCompleted: {
            console.log("=== MarkingRepeater initialized")
        }

        delegate: MarkingLine {
            // Properties are required in MarkingLine.qml and automatically bound to model roles
        }
    }

    // Fitted Lines Points and Segments (3 points per line from fittedLinesModel)
    Repeater3D {
        id: fittedLinesRepeater
        model: fittedLinesModel

        Component.onCompleted: {
            console.log("=== FittedLinesRepeater initialized ===")
        }

        delegate: Node {
            id: fittedLineDelegate

            required property int side
            required property string lineColorName
            required property string lineStyleName
            required property var pointsMeters
            required property bool isValid

            function toFiniteNumber(value, fallback) {
                var n = Number(value);
                return isFinite(n) ? n : (fallback === undefined ? 0 : fallback);
            }

            function pointMetersAt(index) {
                if (!pointsMeters || pointsMeters.length <= index || !pointsMeters[index])
                    return Qt.vector2d(0, 0);
                var p = pointsMeters[index];
                return Qt.vector2d(
                    toFiniteNumber(p.x, 0),
                    toFiniteNumber(p.y, 0)
                );
            }

            readonly property bool hasThreePoints: pointsMeters && pointsMeters.length >= 3
            readonly property var p0: pointMetersAt(0)
            readonly property var p1: pointMetersAt(1)
            readonly property var p2: pointMetersAt(2)

            readonly property string styleLower: String(fittedLineDelegate.lineStyleName || "").toLowerCase()
            readonly property bool isDashedStyle: styleLower === "dashed"
            readonly property bool isDoubleStyle: styleLower === "double"

            // Quadratic Bezier through p0 (t=0), p1 (t=0.5), p2 (t=1).
            readonly property var controlPoint: Qt.vector2d(
                2 * p1.x - 0.5 * (p0.x + p2.x),
                2 * p1.y - 0.5 * (p0.y + p2.y)
            )

            function curvePointAt(t) {
                var tt = Math.max(0.0, Math.min(1.0, Number(t)));
                var s = 1.0 - tt;
                var w0 = s * s;
                var w1 = 2.0 * s * tt;
                var w2 = tt * tt;
                return Qt.vector2d(
                    w0 * p0.x + w1 * controlPoint.x + w2 * p2.x,
                    w0 * p0.y + w1 * controlPoint.y + w2 * p2.y
                );
            }

            readonly property real calculatedRadius: (fittedLineDelegate.side === 1 || fittedLineDelegate.side === 2) ? 1.2 : 0.8
            readonly property color calculatedColor: {
                const lc = String(fittedLineDelegate.lineColorName || "").toLowerCase()
                if (lc === "yellow") return "#ffcc00"
                if (lc === "red") return "#ff4444"
                return "#f0f4f8"
            }
            // Emissive based on line color for consistent glow
            readonly property vector3d calculatedEmissive: Qt.vector3d(
                calculatedColor.r * 0.85,
                calculatedColor.g * 0.85,
                calculatedColor.b * 0.85
            )
            readonly property real pointHeight: carScene.lanePointHeight

            // Point 0
            Node {
                LanePoint {
                    xMeters: -fittedLineDelegate.pointsMeters[0].x || 0
                    yMeters: fittedLineDelegate.pointsMeters[0].y || 0
                    pointColor: fittedLineDelegate.calculatedColor
                    radius: fittedLineDelegate.calculatedRadius
                    height: fittedLineDelegate.pointHeight
                }
            }

            // Point 1
            Node {
                LanePoint {
                    xMeters: -fittedLineDelegate.pointsMeters[1].x || 0
                    yMeters: fittedLineDelegate.pointsMeters[1].y || 0
                    pointColor: fittedLineDelegate.calculatedColor
                    radius: fittedLineDelegate.calculatedRadius
                    height: fittedLineDelegate.pointHeight
                }
            }

            // Point 2
            Node {
                LanePoint {
                    xMeters: -fittedLineDelegate.pointsMeters[2].x || 0
                    yMeters: fittedLineDelegate.pointsMeters[2].y || 0
                    pointColor: fittedLineDelegate.calculatedColor
                    radius: fittedLineDelegate.calculatedRadius
                    height: fittedLineDelegate.pointHeight
                }
            }

            // Curve approximation as many short segments.
            Repeater3D {
                id: fittedLineCurveRepeater
                model: (fittedLineDelegate.isValid && fittedLineDelegate.hasThreePoints && !fittedLineDelegate.isDoubleStyle) ? carScene.fittedLineCurveSegments : 0

                Model {
                    required property int index

                    readonly property real t0: index / carScene.fittedLineCurveSegments
                    readonly property real t1: (index + 1) / carScene.fittedLineCurveSegments

                    readonly property var a: fittedLineDelegate.curvePointAt(t0)
                    readonly property var b: fittedLineDelegate.curvePointAt(t1)

                    readonly property real x1: a.x
                    readonly property real y1: a.y
                    readonly property real x2: b.x
                    readonly property real y2: b.y

                    readonly property real dx: x2 - x1
                    readonly property real dy: y2 - y1
                    readonly property real segmentLength: Math.sqrt(dx * dx + dy * dy)
                    readonly property real angle: Math.atan2(dx, dy) * 180 / Math.PI
                    readonly property bool showSegment: !fittedLineDelegate.isDashedStyle || (index % 2 === 0)

                    visible: showSegment && segmentLength > 0.001
                    source: "#Cube"

                    position: Qt.vector3d(
                        - (x1 + dx / 2) * sceneConfig.scaleFactor,
                        laneLineY,
                        (y1 + dy / 2) * sceneConfig.scaleFactor
                    )

                    scale: Qt.vector3d(
                        laneLineWidth,
                        laneLineHeight,
                        segmentLength
                    )

                    eulerRotation: Qt.vector3d(0, -angle, 0)

                    materials: PrincipledMaterial {
                        baseColor: fittedLineDelegate.calculatedColor
                        roughness: 0.2
                        specularAmount: 0.35
                        metalness: 0.0
                        emissiveFactor: fittedLineDelegate.calculatedEmissive
                    }
                }
            }

            // Double style: two parallel curves offset to both sides.
            Repeater3D {
                id: fittedLineDoubleCurveRepeaterPos
                model: (fittedLineDelegate.isValid && fittedLineDelegate.hasThreePoints && fittedLineDelegate.isDoubleStyle) ? carScene.fittedLineCurveSegments : 0

                Model {
                    required property int index

                    readonly property real t0: index / carScene.fittedLineCurveSegments
                    readonly property real t1: (index + 1) / carScene.fittedLineCurveSegments

                    readonly property var a: fittedLineDelegate.curvePointAt(t0)
                    readonly property var b: fittedLineDelegate.curvePointAt(t1)

                    readonly property real x1: a.x
                    readonly property real y1: a.y
                    readonly property real x2: b.x
                    readonly property real y2: b.y

                    readonly property real dx: x2 - x1
                    readonly property real dy: y2 - y1
                    readonly property real segmentLength: Math.sqrt(dx * dx + dy * dy)
                    readonly property real angle: Math.atan2(dx, dy) * 180 / Math.PI

                    readonly property real offsetAmount: carScene.fittedLineDoubleOffsetMeters * 0.5
                    readonly property real perpX: segmentLength > 0.001 ? (-dy / segmentLength) * offsetAmount : 0
                    readonly property real perpY: segmentLength > 0.001 ? (dx / segmentLength) * offsetAmount : 0

                    visible: segmentLength > 0.001
                    source: "#Cube"

                    position: Qt.vector3d(
                        - (x1 + dx / 2 + perpX) * sceneConfig.scaleFactor,
                        laneLineY,
                        (y1 + dy / 2 + perpY) * sceneConfig.scaleFactor
                    )

                    scale: Qt.vector3d(
                        laneLineWidth * 0.8,
                        laneLineHeight,
                        segmentLength
                    )

                    eulerRotation: Qt.vector3d(0, -angle, 0)

                    materials: PrincipledMaterial {
                        baseColor: fittedLineDelegate.calculatedColor
                        roughness: 0.2
                        specularAmount: 0.35
                        metalness: 0.0
                        emissiveFactor: fittedLineDelegate.calculatedEmissive
                    }
                }
            }

            Repeater3D {
                id: fittedLineDoubleCurveRepeaterNeg
                model: (fittedLineDelegate.isValid && fittedLineDelegate.hasThreePoints && fittedLineDelegate.isDoubleStyle) ? carScene.fittedLineCurveSegments : 0

                Model {
                    required property int index

                    readonly property real t0: index / carScene.fittedLineCurveSegments
                    readonly property real t1: (index + 1) / carScene.fittedLineCurveSegments

                    readonly property var a: fittedLineDelegate.curvePointAt(t0)
                    readonly property var b: fittedLineDelegate.curvePointAt(t1)

                    readonly property real x1: a.x
                    readonly property real y1: a.y
                    readonly property real x2: b.x
                    readonly property real y2: b.y

                    readonly property real dx: x2 - x1
                    readonly property real dy: y2 - y1
                    readonly property real segmentLength: Math.sqrt(dx * dx + dy * dy)
                    readonly property real angle: Math.atan2(dx, dy) * 180 / Math.PI

                    readonly property real offsetAmount: carScene.fittedLineDoubleOffsetMeters * 0.5
                    readonly property real perpX: segmentLength > 0.001 ? (-dy / segmentLength) * offsetAmount : 0
                    readonly property real perpY: segmentLength > 0.001 ? (dx / segmentLength) * offsetAmount : 0

                    visible: segmentLength > 0.001
                    source: "#Cube"

                    position: Qt.vector3d(
                        - (x1 + dx / 2 - perpX) * sceneConfig.scaleFactor,
                        laneLineY,
                        (y1 + dy / 2 - perpY) * sceneConfig.scaleFactor
                    )

                    scale: Qt.vector3d(
                        laneLineWidth * 0.8,
                        laneLineHeight,
                        segmentLength
                    )

                    eulerRotation: Qt.vector3d(0, -angle, 0)

                    materials: PrincipledMaterial {
                        baseColor: fittedLineDelegate.calculatedColor
                        roughness: 0.2
                        specularAmount: 0.35
                        metalness: 0.0
                        emissiveFactor: fittedLineDelegate.calculatedEmissive
                    }
                }
            }
        }
    }

}
