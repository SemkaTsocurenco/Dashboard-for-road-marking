import QtQuick
import QtQuick3D

Node {
    id: root

    required property real xMeters
    required property real yMeters
    required property color pointColor
    property real radius: 0.3
    property real height: 0.05

    // позиционируем общий "узел", а не сам цилиндр
    position: Qt.vector3d(
        xMeters * sceneConfig.scaleFactor,
        0,
        yMeters * sceneConfig.scaleFactor
    )

    // Внешний цилиндр
    Model {
        id: outer
        source: "#Cylinder"

        // чтобы стоял на "земле" (основание на y=0)
        position: Qt.vector3d(0, height / 2, 0)
        scale: Qt.vector3d(radius, height, radius)

        materials: PrincipledMaterial {
            baseColor: pointColor
            emissiveFactor: Qt.vector3d(pointColor.r * 0.4,
                                        pointColor.g * 0.4,
                                        pointColor.b * 0.4)
            roughness: 0.3
            metalness: 0.1
        }
    }

    // Внутренний (фиолетовый) цилиндр
    Model {
        id: inner
        source: "#Cylinder"

        // центр на 1 единицу выше (плюс его половина высоты, чтобы тоже "стоял")
        position: Qt.vector3d(0, 1 + height / 2, 0)

        // радиус вдвое меньше
        scale: Qt.vector3d(radius / 2, height, radius / 2)

        materials: PrincipledMaterial {
            baseColor: "purple"         // или Qt.rgba(0.6, 0.0, 0.9, 1.0)
            emissiveFactor: Qt.vector3d(0.2, 0.0, 0.2)
            roughness: 0.3
            metalness: 0.1
        }
    }
}
