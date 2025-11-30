import QtQuick
import QtQuick3D

Node {
    // Expose camera to parent (Dashboard.qml)
    property alias camera: camera

    // Camera placement tuned for a balanced isometric view
    property real cameraHeight: 250
    property real cameraDistance: -25
    Component.onCompleted: {
        console.log("CarScene loaded")
        console.log("Camera position:", camera.position)
        console.log("Camera rotation:", camera.eulerRotation)
        console.log("markingModel defined:", typeof markingModel !== 'undefined')
        console.log("markingModel:", markingModel)
        if (typeof markingModel !== 'undefined' && markingModel) {
            console.log("Marking model count:", markingModel.count)
        }
    }

    PerspectiveCamera {
        id: camera
        position: Qt.vector3d(0, cameraHeight, cameraDistance) // look toward -Z
        eulerRotation: Qt.vector3d(-35, 0, 0) // tilt downward toward the road
        fieldOfView: 60
        clipNear: 0.1
        clipFar: 1000
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

    // Local light near the car to make it pop (brightness reduced to avoid washing out)
    PointLight {
        position: Qt.vector3d(0, 10.0, 0)
        brightness: 5
        color: "#ffffff"
        quadraticFade: 0.5
        linearFade: 0.1
        constantFade: 1.0
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
        id: markingRepeater
        model: markingModel

        onModelChanged: {
            console.log("Repeater3D model changed, count:", model ? model.count : 0)
        }

        delegate: MarkingLine {}
    }

    // Visualize lane boundary points (if provided) as small markers
    Repeater3D {
        id: leftPointsRepeater
        model: (typeof laneViewModel !== 'undefined' && laneViewModel && laneViewModel.leftPoints) ? laneViewModel.leftPoints : []

        Model {
            required property var modelData
            source: "#Sphere"
            scale: Qt.vector3d(0.08, 0.08, 0.08)
            // Invert Y to match camera view direction
            position: Qt.vector3d(modelData.x, 0.1, -modelData.y)
            materials: PrincipledMaterial {
                baseColor: "#ffd700"
                emissiveFactor: Qt.vector3d(0.3, 0.3, 0.0)
                roughness: 0.4
            }
        }
    }

    Repeater3D {
        id: rightPointsRepeater
        model: (typeof laneViewModel !== 'undefined' && laneViewModel && laneViewModel.rightPoints) ? laneViewModel.rightPoints : []

        Model {
            required property var modelData
            source: "#Sphere"
            scale: Qt.vector3d(0.08, 0.08, 0.08)
            // Invert Y to match camera view direction
            position: Qt.vector3d(modelData.x, 0.1, -modelData.y)
            materials: PrincipledMaterial {
                baseColor: "#ffffff"
                emissiveFactor: Qt.vector3d(0.3, 0.3, 0.3)
                roughness: 0.4
            }
        }
    }
}
