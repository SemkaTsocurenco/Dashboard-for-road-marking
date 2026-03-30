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

    // Gradient sky background — visible through transparent View3D
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0;  color: "#010306" }
            GradientStop { position: 0.45; color: "#060d1a" }
            GradientStop { position: 0.75; color: "#0a1828" }
            GradientStop { position: 1.0;  color: "#0d1f30" }
        }
    }

    View3D {
        id: view3d
        anchors.fill: parent
        camera: carScene.camera  // CRITICAL: Must specify camera!

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Transparent
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.VeryHigh
            // depthFogEnabled: true
            // fogColor: "#060d18"
            // fogNearDistance: 5000
            // fogFarDistance: 14000
            // fogDensity: 1.2  // requires Qt >= 6.5
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

        // ── Distance labels on grid lines ─────────────────────────────────────
        // Grid parameters must match RoadPlane.qml (road centered at Z=0)
        readonly property real _sf:          sceneConfig.scaleFactor
        readonly property real _roadStartZ:  -sceneConfig.roadLength * _sf / 2.0
        readonly property real _roadEndZ:     sceneConfig.roadLength * _sf / 2.0
        readonly property real _gridM:        5.0
        readonly property real _firstM:       Math.ceil((_roadStartZ / _sf) / _gridM) * _gridM
        readonly property real _lastM:        Math.floor((_roadEndZ   / _sf) / _gridM) * _gridM
        readonly property int  _gridCount:    Math.round((_lastM - _firstM) / _gridM) + 1
        // Camera lens world Z (= forward_offset_m * scaleFactor — the nose of the car)
        readonly property real _cameraLensZ:  sceneConfig.fovForwardOffsetM * _sf
        // Label X anchor: negative-X road edge (right side)
        readonly property real _labelXWorld:  (100.0 * sceneConfig.roadWidth / 2.0 + 80.0)
        readonly property real _planeY:       sceneConfig.roadYPosition + 5.0

        // Force labels to refresh once the scene is fully rendered
        property int _refreshTick: 0
        Timer { interval: 150; running: true; repeat: false; onTriggered: parent._refreshTick++ }

        Repeater {
            model: parent._gridCount

            Item {
                required property int index

                readonly property real worldZ:         (parent._firstM + index * parent._gridM) * parent._sf
                // Distance FROM CAMERA (positive = ahead of camera, i.e. in FOV direction)
                readonly property real distFromCamM:   (worldZ - parent._cameraLensZ) / parent._sf
                // Only show labels that are in the camera's forward FOV (ahead of lens)
                readonly property bool inFov:          distFromCamM >= -0.5   // slight tolerance for 0m label

                // Re-evaluate when camera moves or after initial render
                readonly property var screenPos: {
                    let _t  = parent._refreshTick
                    let _cp = carScene.camera.position
                    let _cr = carScene.camera.eulerRotation
                    return view3d.mapFrom3DScene(Qt.vector3d(parent._labelXWorld, parent._planeY, worldZ))
                }

                readonly property bool onScreen: screenPos.x > 2 && screenPos.x < parent.width - 2
                                              && screenPos.y > 2 && screenPos.y < parent.height - 2

                visible: inFov && onScreen

                Text {
                    x: parent.screenPos.x - width / 2
                    y: parent.screenPos.y - height / 2
                    // "0 м" at the camera position, increasing as you go further behind
                    text: parent.distFromCamM.toFixed(0) + " м"
                    color: parent.distFromCamM < 0.5 ? "#ff8800" : '#d3eeff'
                    font.pixelSize: 11
                    font.family: "monospace"
                    opacity: 0.85
                }
            }
        }

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
