import QtQuick
import QtQuick3D

Item {
    id: root

    required property Camera camera

    // Movement speed
    property real moveSpeed: 1.0
    property real rotateSpeed: 2.0

    // Camera state display
    property string positionText: "Pos: (" + camera.position.x.toFixed(1) + ", " +
                                           camera.position.y.toFixed(1) + ", " +
                                           camera.position.z.toFixed(1) + ")"
    property string rotationText: "Rot: (" + camera.eulerRotation.x.toFixed(1) + "°, " +
                                            camera.eulerRotation.y.toFixed(1) + "°, " +
                                            camera.eulerRotation.z.toFixed(1) + "°)"

    focus: true

    Keys.onPressed: (event) => {
        var forward = Qt.vector3d(0, 0, -1);
        var right = Qt.vector3d(1, 0, 0);
        var up = Qt.vector3d(0, 1, 0);

        // Rotate vectors based on camera rotation
        var yaw = camera.eulerRotation.y * Math.PI / 180;
        var cosY = Math.cos(yaw);
        var sinY = Math.sin(yaw);

        var rotatedForward = Qt.vector3d(
            forward.x * cosY - forward.z * sinY,
            0,
            forward.x * sinY + forward.z * cosY
        );

        var rotatedRight = Qt.vector3d(
            right.x * cosY - right.z * sinY,
            0,
            right.x * sinY + right.z * cosY
        );

        switch(event.key) {
            case Qt.Key_W:
                camera.position = Qt.vector3d(
                    camera.position.x + rotatedForward.x * moveSpeed,
                    camera.position.y + rotatedForward.y * moveSpeed,
                    camera.position.z + rotatedForward.z * moveSpeed
                );
                event.accepted = true;
                break;
            case Qt.Key_S:
                camera.position = Qt.vector3d(
                    camera.position.x - rotatedForward.x * moveSpeed,
                    camera.position.y - rotatedForward.y * moveSpeed,
                    camera.position.z - rotatedForward.z * moveSpeed
                );
                event.accepted = true;
                break;
            case Qt.Key_A:
                camera.position = Qt.vector3d(
                    camera.position.x - rotatedRight.x * moveSpeed,
                    camera.position.y - rotatedRight.y * moveSpeed,
                    camera.position.z - rotatedRight.z * moveSpeed
                );
                event.accepted = true;
                break;
            case Qt.Key_D:
                camera.position = Qt.vector3d(
                    camera.position.x + rotatedRight.x * moveSpeed,
                    camera.position.y + rotatedRight.y * moveSpeed,
                    camera.position.z + rotatedRight.z * moveSpeed
                );
                event.accepted = true;
                break;
            case Qt.Key_Space:
                camera.position = Qt.vector3d(
                    camera.position.x,
                    camera.position.y + moveSpeed,
                    camera.position.z
                );
                event.accepted = true;
                break;
            case Qt.Key_Shift:
                camera.position = Qt.vector3d(
                    camera.position.x,
                    camera.position.y - moveSpeed,
                    camera.position.z
                );
                event.accepted = true;
                break;
            case Qt.Key_Up:
                camera.eulerRotation = Qt.vector3d(
                    camera.eulerRotation.x - rotateSpeed,
                    camera.eulerRotation.y,
                    camera.eulerRotation.z
                );
                event.accepted = true;
                break;
            case Qt.Key_Down:
                camera.eulerRotation = Qt.vector3d(
                    camera.eulerRotation.x + rotateSpeed,
                    camera.eulerRotation.y,
                    camera.eulerRotation.z
                );
                event.accepted = true;
                break;
            case Qt.Key_Left:
                camera.eulerRotation = Qt.vector3d(
                    camera.eulerRotation.x,
                    camera.eulerRotation.y - rotateSpeed,
                    camera.eulerRotation.z
                );
                event.accepted = true;
                break;
            case Qt.Key_Right:
                camera.eulerRotation = Qt.vector3d(
                    camera.eulerRotation.x,
                    camera.eulerRotation.y + rotateSpeed,
                    camera.eulerRotation.z
                );
                event.accepted = true;
                break;
            case Qt.Key_R:
                // Reset to default position
                camera.position = Qt.vector3d(0, 15, -25);
                camera.eulerRotation = Qt.vector3d(-35, 0, 0);
                event.accepted = true;
                break;
        }

    }
}
