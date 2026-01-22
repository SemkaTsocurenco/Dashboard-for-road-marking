import QtQuick
import QtQuick3D

Item {
    id: root

    required property Camera camera

    // Movement speed
    property real moveSpeed: 1.0
    property real rotateSpeed: 3.0
    property bool moveForward: false
    property bool moveBackward: false
    property bool moveLeft: false
    property bool moveRight: false
    property bool moveUp: false
    property bool moveDown: false
    property bool rotateUp: false
    property bool rotateDown: false
    property bool rotateLeft: false
    property bool rotateRight: false

    // Camera state display
    property string positionText: "Pos: (" + camera.position.x.toFixed(1) + ", " +
                                           camera.position.y.toFixed(1) + ", " +
                                           camera.position.z.toFixed(1) + ")"
    property string rotationText: "Rot: (" + camera.eulerRotation.x.toFixed(1) + "°, " +
                                            camera.eulerRotation.y.toFixed(1) + "°, " +
                                            camera.eulerRotation.z.toFixed(1) + "°)"

    focus: true
    onActiveFocusChanged: {
        if (!activeFocus) {
            moveForward = false;
            moveBackward = false;
            moveLeft = false;
            moveRight = false;
            moveUp = false;
            moveDown = false;
            rotateUp = false;
            rotateDown = false;
            rotateLeft = false;
            rotateRight = false;
        }
    }

    function setKeyState(key, isDown) {
        switch (key) {
            case Qt.Key_W:
                moveForward = isDown;
                return true;
            case Qt.Key_S:
                moveBackward = isDown;
                return true;
            case Qt.Key_A:
                moveLeft = isDown;
                return true;
            case Qt.Key_D:
                moveRight = isDown;
                return true;
            case Qt.Key_Space:
                moveUp = isDown;
                return true;
            case Qt.Key_Shift:
                moveDown = isDown;
                return true;
            case Qt.Key_Up:
                rotateUp = isDown;
                return true;
            case Qt.Key_Down:
                rotateDown = isDown;
                return true;
            case Qt.Key_Left:
                rotateLeft = isDown;
                return true;
            case Qt.Key_Right:
                rotateRight = isDown;
                return true;
        }

        return false;
    }

    Keys.onPressed: (event) => {
        if (event.isAutoRepeat) {
            return;
        }

        if (setKeyState(event.key, true)) {
            event.accepted = true;
            return;
        }

        if (event.key === Qt.Key_R) {
            // Reset to default position
            camera.position = Qt.vector3d(0, 15, -25);
            camera.eulerRotation = Qt.vector3d(-35, 0, 0);
            event.accepted = true;
        }
    }

    Keys.onReleased: (event) => {
        if (event.isAutoRepeat) {
            return;
        }

        if (setKeyState(event.key, false)) {
            event.accepted = true;
        }
    }

    Timer {
        id: movementTimer
        interval: 16
        repeat: true
        running: true
        triggeredOnStart: true

        onTriggered: {
            var scale = interval / 33.0;
            var moveStep = 40 * moveSpeed * scale;
            var verticalStep = (moveSpeed + 50) * scale;

            // Rotate vectors based on camera rotation (right-handed, Y-up).
            var yaw = camera.eulerRotation.y * Math.PI / 180;
            var cosY = Math.cos(yaw);
            var sinY = Math.sin(yaw);

            var forward = Qt.vector3d(0, 0, -moveStep);
            var right = Qt.vector3d(moveStep, 0, 0);

            var rotatedForward = Qt.vector3d(
                forward.x * cosY + forward.z * sinY,
                0,
                -forward.x * sinY + forward.z * cosY
            );

            var rotatedRight = Qt.vector3d(
                right.x * cosY + right.z * sinY,
                0,
                -right.x * sinY + right.z * cosY
            );

            var dx = 0;
            var dy = 0;
            var dz = 0;

            if (moveForward) {
                dx += rotatedForward.x;
                dz += rotatedForward.z;
            }
            if (moveBackward) {
                dx -= rotatedForward.x;
                dz -= rotatedForward.z;
            }
            if (moveLeft) {
                dx -= rotatedRight.x;
                dz -= rotatedRight.z;
            }
            if (moveRight) {
                dx += rotatedRight.x;
                dz += rotatedRight.z;
            }
            if (moveUp) {
                dy += verticalStep;
            }
            if (moveDown) {
                dy -= verticalStep;
            }

            if (dx || dy || dz) {
                camera.position = Qt.vector3d(
                    camera.position.x + dx,
                    camera.position.y + dy,
                    camera.position.z + dz
                );
            }

            var rotateStep = rotateSpeed * scale;
            var pitchDelta = 0;
            var yawDelta = 0;

            if (rotateUp) {
                pitchDelta += rotateStep;
            }
            if (rotateDown) {
                pitchDelta -= rotateStep;
            }
            if (rotateLeft) {
                yawDelta += rotateStep;
            }
            if (rotateRight) {
                yawDelta -= rotateStep;
            }

            if (pitchDelta || yawDelta) {
                camera.eulerRotation = Qt.vector3d(
                    camera.eulerRotation.x + pitchDelta,
                    camera.eulerRotation.y + yawDelta,
                    camera.eulerRotation.z
                );
            }
        }
    }
}
