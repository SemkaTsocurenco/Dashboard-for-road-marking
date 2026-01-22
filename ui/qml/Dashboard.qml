import QtQuick
import QtQuick3D
import Theme 1.0

Item {
    id: root
    width: sceneConfig.dashboardWidth
    height: sceneConfig.dashboardHeight

    Component.onCompleted: {

        cameraController.focus = true
    }

    View3D {
        id: view3d
        anchors.fill: parent
        camera: carScene.camera  // CRITICAL: Must specify camera!

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "#0a0c0f"
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.VeryHigh
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
        // Rectangle {
        //     anchors.top: parent.top
        //     anchors.right: parent.right
        //     anchors.margins: Theme.spacingXLarge
        //     width: sceneConfig.cameraInfoWidth
        //     height: sceneConfig.cameraInfoHeight
        //     color: Theme.bgSurface1
        //     opacity: Theme.overlayOpacity
        //     radius: Theme.radiusMedium
        //     border.color: Theme.border
        //     border.width: 1

        //     Column {
        //         anchors.fill: parent
        //         anchors.margins: Theme.spacingMedium
        //         spacing: Theme.spacingSmall

        //         Text {
        //             text: cameraController.positionText
        //             color: Theme.textSecondary
        //             font.pixelSize: Theme.fontMedium
        //             font.family: Theme.fontFamilyMono
        //         }
        //         Text {
        //             text: cameraController.rotationText
        //             color: Theme.textSecondary
        //             font.pixelSize: Theme.fontMedium
        //             font.family: Theme.fontFamilyMono
        //         }
        //         Text {
        //             text: "WASD+Space/Shift: move | Arrows: rotate | R: reset"
        //             color: Theme.textDisabled
        //             font.pixelSize: Theme.fontXSmall
        //         }
        //     }
        // }

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
