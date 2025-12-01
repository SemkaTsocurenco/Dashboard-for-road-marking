import QtQuick
import QtQuick3D

Item {
    id: root
    width: 800
    height: 600

    Component.onCompleted: {

        cameraController.focus = true
    }

    View3D {
        id: view3d
        anchors.fill: parent
        camera: carScene.camera  // CRITICAL: Must specify camera!

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "#1a1a1a"
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        CarScene {
            id: carScene
        }
    }

    // Camera controller
    CameraController {
        id: cameraController
        anchors.fill: parent
        camera: carScene.camera
    }

    // Overlays
    Item {
        anchors.fill: parent
        anchors.margins: 0
        z: 10

        // Camera position display
        Rectangle {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 10
            width: 350
            height: 80
            color: "#80000000"
            radius: 5

            Column {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 5

                Text {
                    text: cameraController.positionText
                    color: "#00ff00"
                    font.pixelSize: 14
                    font.family: "monospace"
                }
                Text {
                    text: cameraController.rotationText
                    color: "#00ff00"
                    font.pixelSize: 14
                    font.family: "monospace"
                }
                Text {
                    text: "WASD+Space/Shift: move | Arrows: rotate | R: reset"
                    color: "#ffff00"
                    font.pixelSize: 10
                }
            }
        }

        WarningPanel {
            anchors.top: parent.top
            anchors.left: parent.left
        }

        CenterOffsetIndicator {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
        }

        ConnectionStatus {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
        }
    }
}
