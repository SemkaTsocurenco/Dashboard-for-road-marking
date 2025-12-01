import QtQuick
import QtQuick3D

Node {
    // Expose camera to parent (Dashboard.qml)
    property alias camera: camera

    // Camera placement tuned for a balanced isometric view
    property real cameraHeight: sceneConfig.cameraHeight
    property real cameraDistance: sceneConfig.cameraDistance
    Component.onCompleted: {
    }

    PerspectiveCamera {
        id: camera
        position: Qt.vector3d(0, cameraHeight, cameraDistance) // look toward -Z
        eulerRotation: Qt.vector3d(sceneConfig.cameraPitchAngle, 0, 0) // tilt downward toward the road
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
        // Lift slightly above the plane so it is clearly visible
        position: Qt.vector3d(0, sceneConfig.carYElevation, 0)
    }


    Repeater3D {
        id: markingRepeater
        model: markingModel

        onModelChanged: {
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
            scale: Qt.vector3d(sceneConfig.markerScale, sceneConfig.markerScale, sceneConfig.markerScale)
            // Invert Y to match camera view direction
            position: Qt.vector3d(modelData.x, sceneConfig.markerYPosition, -modelData.y)
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
            scale: Qt.vector3d(sceneConfig.markerScale, sceneConfig.markerScale, sceneConfig.markerScale)
            // Invert Y to match camera view direction
            position: Qt.vector3d(modelData.x, sceneConfig.markerYPosition, -modelData.y)
            materials: PrincipledMaterial {
                baseColor: "#ffffff"
                emissiveFactor: Qt.vector3d(0.3, 0.3, 0.3)
                roughness: 0.4
            }
        }
    }
}
