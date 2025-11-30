import QtQuick
import QtQuick3D

Rectangle {
    anchors.fill: parent
    color: "#1a1a1a"
    Component.onCompleted: console.log("Dashboard.qml loaded")

    View3D {
        anchors.fill: parent
        camera: carScene.camera
        z: 0

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

    // Overlays
    Item {
        anchors.fill: parent
        anchors.margins: 0
        z: 10

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
