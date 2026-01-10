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


    // Repeater3D {
    //     id: markingRepeater
    //     model: markingModel

    //     onModelChanged: {
    //     }

    //     delegate: MarkingLine {}
    // }

    // Visualize lane boundary points (if provided) as small markers
    Repeater3D {
        id: leftPointsRepeater
        model:  laneViewModel.leftPoints

        Model {
            required property point modelData
            source: "#Sphere"
            scale: Qt.vector3d(1,1,1)
            Component.onCompleted: console.log("Left marker scale:", sceneConfig.markerScale)
            // Invert Y to match camera view direction and apply scale_factor
            position: Qt.vector3d(
                modelData.x * sceneConfig.scaleFactor,
                0,
                modelData.y * sceneConfig.scaleFactor
            )
            onPositionChanged: {
                console.log("Updated position - x:", position.x, "y:", position.y, "z:", position.z)
            }

            materials: PrincipledMaterial {
                baseColor: "#ffd700"
                emissiveFactor: Qt.vector3d(0.3, 0.3, 0.0)
                roughness: 0.4
            }
        }
    }

    Repeater3D {
        id: rightPointsRepeater
        model: laneViewModel.rightPoints 

        Model {
            required property var modelData
            source: "#Sphere"
            scale: Qt.vector3d(1,1,1)
            Component.onCompleted: console.log("Left marker scale:", sceneConfig.markerScale)
            // Invert Y to match camera view direction and apply scale_factor
            position: Qt.vector3d(
                modelData.x * sceneConfig.scaleFactor,
                0,
                modelData.y * sceneConfig.scaleFactor
            )
            materials: PrincipledMaterial {
                baseColor: "#ffffff"
                emissiveFactor: Qt.vector3d(0.3, 0.3, 0.3)
                roughness: 0.4
            }

        }
    }
}
