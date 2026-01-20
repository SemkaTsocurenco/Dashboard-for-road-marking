import QtQuick
import QtQuick3D

Node {
    // Expose camera to parent (Dashboard.qml)
    property alias camera: camera

    // Camera placement tuned for a balanced isometric view
    property real cameraHeight: sceneConfig.cameraHeight
    property real cameraDistance: sceneConfig.cameraDistance

    function laneColorFromString(colorName) {
        const c = String(colorName || "").toLowerCase()
        if (c === "yellow") return "#ffd700"
        if (c === "red") return "#ff5555"
        return "#ffffff"
    }

    function orderedLanePoints(points) {
        var ordered = []
        if (!points || points.length === 0)
            return ordered
        for (var i = 0; i < points.length; ++i) {
            var p = points[i]
            if (!p || !isFinite(p.x) || !isFinite(p.y))
                continue
            ordered.push({x: p.x, y: p.y, d2: p.x * p.x + p.y * p.y})
        }
        ordered.sort(function(a, b) { return b.d2 - a.d2 })
        return ordered
    }

    function laneSegments(points) {
        var ordered = orderedLanePoints(points)
        var segments = []
        if (ordered.length >= 2) {
            segments.push({x1: ordered[0].x, y1: ordered[0].y, x2: ordered[1].x, y2: ordered[1].y})
        }
        if (ordered.length >= 3) {
            segments.push({x1: ordered[1].x, y1: ordered[1].y, x2: ordered[2].x, y2: ordered[2].y})
        }
        return segments
    }

    readonly property color leftLaneColor: laneColorFromString(laneViewModel.laneColorLeft)
    readonly property color rightLaneColor: laneColorFromString(laneViewModel.laneColorRight)
    readonly property real lanePointRadius: 0.95
    readonly property real lanePointHeight: 0.05
    readonly property real laneLineWidth: sceneConfig.edgeLineWidth
    readonly property real laneLineHeight: sceneConfig.roadThickness * 2
    readonly property real laneLineY: lanePointHeight * 0.5
    readonly property var leftLaneSegments: laneSegments(laneViewModel.leftPoints)
    readonly property var rightLaneSegments: laneSegments(laneViewModel.rightPoints)

    PerspectiveCamera {
        id: camera
        position: Qt.vector3d(0, cameraHeight, cameraDistance) // look toward -Z
        eulerRotation: Qt.vector3d(sceneConfig.cameraPitchAngle, 180, 0) // tilt downward toward the road
        fieldOfView: 60
        clipNear: sceneConfig.cameraClipNear
        clipFar: sceneConfig.cameraClipFar
    }

    DirectionalLight {
        eulerRotation: Qt.vector3d(-45, 15, 0)
        brightness: sceneConfig.mainLightBrightness
        castsShadow: false
        color: "#e0e0e0"
    }

    DirectionalLight {
        eulerRotation: Qt.vector3d(35, -35, 0)
        brightness: sceneConfig.secondaryLightBrightness
        castsShadow: false
        color: "#cfd8e3"
    }

    DirectionalLight {
        eulerRotation: Qt.vector3d(-15, -150, 0)
        brightness: sceneConfig.tertiaryLightBrightness
        castsShadow: false
        color: "#5a5a5a"
    }

    // Local light near the car to make it pop (brightness reduced to avoid washing out)
    PointLight {
        position: Qt.vector3d(0, sceneConfig.pointLightYPosition, 0)
        brightness: sceneConfig.pointLightBrightness
        color: "#ffffff"
        quadraticFade: sceneConfig.pointLightQuadraticFade
        linearFade: sceneConfig.pointLightLinearFade
        constantFade: sceneConfig.pointLightConstantFade
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

        delegate: MarkingLine {
            // Required properties are automatically bound to model roles
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
        }
    }

    // Left lane segments (farthest -> middle -> nearest)
    Repeater3D {
        id: leftSegmentsRepeater
        model: leftLaneSegments

        delegate: Model {
            required property var modelData

            readonly property real x1: modelData.x1
            readonly property real y1: modelData.y1
            readonly property real x2: modelData.x2
            readonly property real y2: modelData.y2
            readonly property real dx: x2 - x1
            readonly property real dy: y2 - y1
            readonly property real segmentLength: Math.sqrt(dx * dx + dy * dy)
            readonly property real angle: Math.atan2(dx, dy) * 180 / Math.PI

            visible: segmentLength > 0.001

            source: "#Cube"

            position: Qt.vector3d(
                (x1 + dx / 2) * sceneConfig.scaleFactor,
                laneLineY,
                (y1 + dy / 2) * sceneConfig.scaleFactor
            )

            scale: Qt.vector3d(
                laneLineWidth,
                laneLineHeight,
                segmentLength * sceneConfig.scaleFactor
            )

            eulerRotation: Qt.vector3d(0, angle, 0)

            materials: PrincipledMaterial {
                baseColor: leftLaneColor
                roughness: 0.75
                specularAmount: 0.0
                emissiveFactor: Qt.vector3d(0.5, 0.5, 0.5)
            }
        }
    }

    // Left lane points (colored pucks)
    Repeater3D {
        id: leftPointsRepeater
        model: laneViewModel.leftPoints



        delegate: LanePoint {
            required property point modelData

            xMeters: modelData.x
            yMeters: modelData.y
            pointColor: leftLaneColor
            radius: lanePointRadius
            height: lanePointHeight
        }
    }

    // Right lane segments (farthest -> middle -> nearest)
    Repeater3D {
        id: rightSegmentsRepeater
        model: rightLaneSegments

        delegate: Model {
            required property var modelData

            readonly property real x1: modelData.x1
            readonly property real y1: modelData.y1
            readonly property real x2: modelData.x2
            readonly property real y2: modelData.y2
            readonly property real dx: x2 - x1
            readonly property real dy: y2 - y1
            readonly property real segmentLength: Math.sqrt(dx * dx + dy * dy)
            readonly property real angle: Math.atan2(dx, dy) * 180 / Math.PI

            visible: segmentLength > 0.001

            source: "#Cube"

            position: Qt.vector3d(
                (x1 + dx / 2) * sceneConfig.scaleFactor,
                laneLineY,
                (y1 + dy / 2) * sceneConfig.scaleFactor
            )

            scale: Qt.vector3d(
                laneLineWidth,
                laneLineHeight,
                segmentLength * sceneConfig.scaleFactor
            )

            eulerRotation: Qt.vector3d(0, angle, 0)

            materials: PrincipledMaterial {
                baseColor: rightLaneColor
                roughness: 0.75
                specularAmount: 0.0
                emissiveFactor: Qt.vector3d(0.5, 0.5, 0.5)
            }
        }
    }

    // Right lane points (colored pucks based on lane color)
    Repeater3D {
        id: rightPointsRepeater
        model: laneViewModel.rightPoints

        delegate: LanePoint {
            required property point modelData

            xMeters: modelData.x
            yMeters: modelData.y
            pointColor: rightLaneColor
            radius: lanePointRadius
            height: lanePointHeight
        }
    }
}
