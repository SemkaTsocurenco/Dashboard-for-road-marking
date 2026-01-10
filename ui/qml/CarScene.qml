import QtQuick
import QtQuick3D

Node {
    // Expose camera to parent (Dashboard.qml)
    property alias camera: camera

    // Camera placement tuned for a balanced isometric view
    property real cameraHeight: sceneConfig.cameraHeight
    property real cameraDistance: sceneConfig.cameraDistance

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

    // Fitted Lines (polynomial curves)
    Repeater3D {
        id: fittedLinesRepeater
        model: fittedLinesModel

        delegate: PolyLine3D {
            // Required properties from model (in decimeters)
            required property real polyA
            required property real polyB
            required property real polyC
            required property int yStart
            required property int yEnd
            required property string lineColorName
            required property string lineStyleName
            required property bool isValid
            required property var pointsMeters
            points: pointsMeters

            // Use internal names for PolyLine3D properties
            yStartMeters: yStart / 10.0
            yEndMeters: yEnd / 10.0

            lineColor: {
                const c = lineColorName.toLowerCase()
                if (c === "yellow") return "#ffd700"
                if (c === "red") return "#ff5555"
                if (c === "white") return "#ffffff"
                return "#ffffff"
            }
            lineStyle: lineStyleName
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
            pointColor: {
                const c = laneViewModel.laneColorLeft.toLowerCase()
                if (c === "yellow") return "#ffd700"
                if (c === "red") return "#ff5555"
                return "#ffffff"
            }
            radius: 0.25
            height: 0.05
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
            pointColor: {
                const c = laneViewModel.laneColorRight.toLowerCase()
                if (c === "yellow") return "#ffd700"
                if (c === "red") return "#ff5555"
                return "#ffffff"
            }
            radius: 0.25
            height: 0.05
        }
    }
}
