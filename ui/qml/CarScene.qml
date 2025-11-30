import QtQuick
import QtQuick3D

Node {
    // Expose camera to parent (Dashboard.qml)
    property alias camera: camera

    // Camera placement tuned for a balanced isometric view
    property real cameraHeight: 50
    property real cameraDistance: -1100
    Component.onCompleted: console.log("CarScene loaded")

    PerspectiveCamera {
        id: camera
        position: Qt.vector3d(0, cameraHeight, cameraDistance) // look toward -Z
        eulerRotation: Qt.vector3d(-50, 0, 0) // tilt downward toward the road
        fieldOfView: 58
        clipNear: 0.1
        clipFar: 120
    }

    DirectionalLight {
        eulerRotation: Qt.vector3d(-45, 15, 0)
        brightness: 0.7
        castsShadow: false
        color: "#e0e0e0"
    }

    DirectionalLight {
        eulerRotation: Qt.vector3d(35, -35, 0)
        brightness: 0.45
        castsShadow: false
        color: "#cfd8e3"
    }

    DirectionalLight {
        eulerRotation: Qt.vector3d(-15, -150, 0)
        brightness: 0.25
        castsShadow: false
        color: "#5a5a5a"
    }

    // Local light near the car to make it pop
    PointLight {
        position: Qt.vector3d(0, 4.0, 4.0)
        brightness: 120
        color: "#ffffff"
        quadraticFade: 0.08
        linearFade: 0.02
        constantFade: 0.5
    }

    RoadPlane {
        widthMeters: 18.0
        lengthMeters: 40.0
    }

    CarModel {
        // Lift slightly above the plane so it is clearly visible
        position: Qt.vector3d(0, 0.6, 0)
    }

    Repeater3D {
        model: markingModel

        delegate: MarkingLine {
            xMeters: model.xMeters
            yMeters: model.yMeters
            lengthMeters: model.lengthMeters
            widthMeters: model.widthMeters
            yawDeg: model.yawDeg
            className: model.className
            isCrosswalk: model.isCrosswalk
            isArrow: model.isArrow
            isValid: model.isValid
            confidence: model.confidence
        }
    }
}
