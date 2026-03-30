# QML Компоненты

## Обзор

3D визуализация реализована с использованием Qt Quick 3D. QML компоненты находятся в директории `/ui/qml/`.

## Структура файлов

```
ui/qml/
├── Dashboard.qml           # Корневой компонент
├── CarScene.qml            # 3D сцена с дорогой и машиной
├── MarkingLine.qml         # Линия разметки (3D)
├── PolyLine3D.qml          # Полиномиальная линия (3D)
├── RoadPlane.qml           # Плоскость дороги
├── CameraController.qml    # Управление камерой
├── WarningPanel.qml        # Панель предупреждений
├── CenterOffsetIndicator.qml # Индикатор смещения
└── Theme/
    └── Theme.qml           # Тема и цвета
```

---

## Dashboard.qml

**Назначение:** Корневой компонент, объединяющий все элементы визуализации.

### Структура

```qml
import QtQuick
import QtQuick3D

Item {
    id: root

    // Ссылки на ViewModel'ы (из C++)
    property var laneViewModel
    property var markingListModel
    property var warningListModel
    property var sceneConfig

    // 3D сцена
    View3D {
        id: view3D
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#1a1a2e"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        // Сцена с дорогой и машиной
        CarScene {
            id: carScene
            laneViewModel: root.laneViewModel
            markingListModel: root.markingListModel
            sceneConfig: root.sceneConfig
        }
    }

    // Панель предупреждений (2D overlay)
    WarningPanel {
        id: warningPanel
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
        model: root.warningListModel
    }

    // Индикатор смещения от центра
    CenterOffsetIndicator {
        id: offsetIndicator
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        offset: laneViewModel ? laneViewModel.centerOffset : 0
        config: root.sceneConfig
    }
}
```

### Свойства

| Свойство | Тип | Описание |
|----------|-----|----------|
| `laneViewModel` | LaneStateViewModel* | ViewModel состояния полос |
| `markingListModel` | MarkingObjectListModel* | Модель объектов разметки |
| `warningListModel` | WarningListModel* | Модель предупреждений |
| `sceneConfig` | SceneConfigViewModel* | Конфигурация сцены |

---

## CarScene.qml

**Назначение:** 3D сцена с изометрическим видом дороги, линиями разметки и моделью автомобиля.

### Структура

```qml
import QtQuick3D

Node {
    id: root

    property var laneViewModel
    property var markingListModel
    property var sceneConfig

    // Масштаб сцены (1м = scaleFactor единиц)
    readonly property real scaleFactor: sceneConfig ? sceneConfig.scaleFactor : 100

    // Камера (изометрический вид)
    PerspectiveCamera {
        id: camera
        position: Qt.vector3d(
            sceneConfig.cameraHeight,
            sceneConfig.cameraHeight,
            sceneConfig.cameraDistance
        )
        eulerRotation: Qt.vector3d(
            sceneConfig.cameraPitchAngle,
            153,  // Yaw для изометрии
            0
        )
        clipNear: sceneConfig.clipNear
        clipFar: sceneConfig.clipFar
    }

    // Освещение
    DirectionalLight {
        id: mainLight
        brightness: sceneConfig.mainLightBrightness
        eulerRotation: Qt.vector3d(-45, -45, 0)
    }

    DirectionalLight {
        id: secondaryLight
        brightness: sceneConfig.secondaryLightBrightness
        eulerRotation: Qt.vector3d(-45, 45, 0)
    }

    // Дорожное полотно
    RoadPlane {
        id: road
        width: sceneConfig.roadWidthM * scaleFactor
        length: sceneConfig.roadLengthM * scaleFactor
    }

    // Левая линия разметки
    MarkingLine {
        id: leftLine
        visible: laneViewModel && laneViewModel.valid
        xPosition: laneViewModel ? laneViewModel.leftOffset * scaleFactor : 0
        lineType: laneViewModel ? laneViewModel.laneTypeLeft : 0
        lineColor: laneViewModel ? laneViewModel.laneColorLeft : 0
        confidence: laneViewModel ? laneViewModel.leftConfidence : 0
        scaleFactor: root.scaleFactor
        config: root.sceneConfig
    }

    // Правая линия разметки
    MarkingLine {
        id: rightLine
        visible: laneViewModel && laneViewModel.valid
        xPosition: laneViewModel ? laneViewModel.rightOffset * scaleFactor : 0
        lineType: laneViewModel ? laneViewModel.laneTypeRight : 0
        lineColor: laneViewModel ? laneViewModel.laneColorRight : 0
        confidence: laneViewModel ? laneViewModel.rightConfidence : 0
        scaleFactor: root.scaleFactor
        config: root.sceneConfig
    }

    // Объекты разметки (пешеходные переходы, стрелки)
    Repeater3D {
        model: root.markingListModel

        delegate: MarkingObject3D {
            xPosition: model.xMeters * scaleFactor
            yPosition: model.yMeters * scaleFactor
            objectWidth: model.widthMeters * scaleFactor
            objectHeight: model.heightMeters * scaleFactor
            classId: model.classId
            confidence: model.confidence
        }
    }

    // 3D модель автомобиля
    Model {
        id: carModel
        source: "#Cube"  // или GLB модель
        position: Qt.vector3d(0, sceneConfig.carYElevation * scaleFactor, 0)
        scale: Qt.vector3d(0.5, 0.3, 1.0)

        materials: PrincipledMaterial {
            baseColor: "#4a90d9"
            metalness: 0.3
            roughness: 0.4
        }
    }
}
```

### Координатная система 3D сцены

```
       +Y (вверх)
        │
        │
        │    +Z (вперёд/от камеры)
        │   ╱
        │  ╱
        │ ╱
        ├──────────→ +X (вправо)
       ╱
      ╱
     ╱
Камера
```

---

## MarkingLine.qml

**Назначение:** 3D представление линии дорожной разметки.

### Структура

```qml
import QtQuick3D

Node {
    id: root

    property real xPosition: 0
    property int lineType: 0      // 0=unknown, 1=solid, 2=dashed, 3=double
    property int lineColor: 0     // 0=unknown, 1=white, 2=yellow, 3=red
    property real confidence: 0
    property real scaleFactor: 100
    property var config

    // Вычисляемые свойства
    readonly property color displayColor: {
        switch (lineColor) {
            case 1: return "white"
            case 2: return "#FFD700"  // Yellow
            case 3: return "#FF4444"  // Red
            default: return "gray"
        }
    }

    readonly property real lineWidth: config ? config.edgeLineWidth * scaleFactor : 15
    readonly property real lineLength: config ? config.roadLengthM * scaleFactor : 10000

    // Сплошная линия
    Model {
        id: solidLine
        visible: lineType === 1 || lineType === 3
        source: "#Cube"
        position: Qt.vector3d(xPosition, config.markingYPosition, lineLength / 2)
        scale: Qt.vector3d(lineWidth, config.markingHeight, lineLength)

        materials: PrincipledMaterial {
            baseColor: displayColor
            opacity: Math.max(config.minOpacity,
                             confidence / config.confidenceDivisor * config.maxOpacity)
        }
    }

    // Прерывистая линия (несколько сегментов)
    Repeater3D {
        model: lineType === 2 ? Math.floor(lineLength / 600) : 0

        Model {
            source: "#Cube"
            position: Qt.vector3d(
                xPosition,
                config.markingYPosition,
                index * 600 + 150
            )
            scale: Qt.vector3d(lineWidth, config.markingHeight, 300)

            materials: PrincipledMaterial {
                baseColor: displayColor
                opacity: confidence / 100 * config.maxOpacity
            }
        }
    }

    // Двойная линия
    Model {
        id: doubleLineOuter
        visible: lineType === 3
        source: "#Cube"
        position: Qt.vector3d(xPosition + lineWidth * 1.5, config.markingYPosition, lineLength / 2)
        scale: Qt.vector3d(lineWidth, config.markingHeight, lineLength)

        materials: PrincipledMaterial {
            baseColor: displayColor
            opacity: confidence / 100 * config.maxOpacity
        }
    }
}
```

### Типы линий

| Тип | Значение | Визуализация |
|-----|----------|--------------|
| Unknown | 0 | Не отображается |
| Solid | 1 | Сплошная линия |
| Dashed | 2 | Прерывистая линия |
| Double | 3 | Двойная сплошная |

---

## PolyLine3D.qml

**Назначение:** Полиномиальная линия разметки (криволинейная).

### Структура

```qml
import QtQuick3D

Node {
    id: root

    property real polyA: 0      // Коэффициент при y²
    property real polyB: 0      // Коэффициент при y
    property real polyC: 0      // Свободный член
    property real confidence: 0
    property color lineColor: "white"
    property int pointCount: 50
    property real yStart: 0
    property real yEnd: 100
    property real scaleFactor: 100
    property var config

    // Генерация точек линии
    function calculateX(y) {
        return polyA * y * y + polyB * y + polyC
    }

    // Сегменты линии
    Repeater3D {
        model: pointCount - 1

        Model {
            id: segment
            source: "#Cylinder"

            property real y1: yStart + index * (yEnd - yStart) / (pointCount - 1)
            property real y2: yStart + (index + 1) * (yEnd - yStart) / (pointCount - 1)
            property real x1: calculateX(y1) * scaleFactor
            property real x2: calculateX(y2) * scaleFactor
            property real z1: y1 * scaleFactor
            property real z2: y2 * scaleFactor

            // Позиция центра сегмента
            position: Qt.vector3d(
                (x1 + x2) / 2,
                config.markingYPosition,
                (z1 + z2) / 2
            )

            // Длина и поворот сегмента
            property real segmentLength: Math.sqrt(
                Math.pow(x2 - x1, 2) + Math.pow(z2 - z1, 2)
            )

            scale: Qt.vector3d(
                config.edgeLineWidth * scaleFactor * 0.5,
                segmentLength / 2,
                config.edgeLineWidth * scaleFactor * 0.5
            )

            eulerRotation: Qt.vector3d(
                90,
                0,
                Math.atan2(x2 - x1, z2 - z1) * 180 / Math.PI
            )

            materials: PrincipledMaterial {
                baseColor: lineColor
                opacity: confidence / 100
            }
        }
    }
}
```

---

## RoadPlane.qml

**Назначение:** Плоскость дорожного полотна.

### Структура

```qml
import QtQuick3D

Model {
    id: root

    property real width: 1200
    property real length: 10000
    property real thickness: 1

    source: "#Cube"
    position: Qt.vector3d(0, -thickness / 2, length / 2)
    scale: Qt.vector3d(width, thickness, length)

    materials: PrincipledMaterial {
        baseColor: "#2a2a2a"  // Тёмно-серый асфальт
        roughness: 0.9
        metalness: 0.0
    }
}
```

---

## CenterOffsetIndicator.qml

**Назначение:** Индикатор смещения автомобиля от центра полосы.

### Структура

```qml
import QtQuick

Rectangle {
    id: root

    property real offset: 0        // Смещение в метрах
    property var config

    width: config ? config.centerOffsetWidth : 320
    height: config ? config.centerOffsetHeight : 130
    color: "#80000000"
    radius: 10

    // Максимальное отображаемое смещение
    readonly property real maxOffset: config ? config.maxOffsetM : 1.0
    readonly property real safeThreshold: config ? config.safeThresholdM : 0.3
    readonly property real criticalThreshold: config ? config.criticalThresholdM : 0.6

    // Цвет индикатора
    readonly property color indicatorColor: {
        var absOffset = Math.abs(offset)
        if (absOffset <= safeThreshold) return "#4CAF50"      // Зелёный
        if (absOffset <= criticalThreshold) return "#FF9800"  // Жёлтый
        return "#F44336"                                       // Красный
    }

    Column {
        anchors.centerIn: parent
        spacing: 10

        // Заголовок
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Смещение от центра"
            color: "white"
            font.pixelSize: 14
        }

        // Шкала
        Rectangle {
            id: scale
            width: root.width - 40
            height: 30
            color: "#333"
            radius: 5

            // Зоны
            Rectangle {
                // Безопасная зона (центр)
                anchors.centerIn: parent
                width: parent.width * (safeThreshold / maxOffset)
                height: parent.height
                color: "#4CAF5040"
                radius: 5
            }

            // Индикатор текущего положения
            Rectangle {
                id: indicator
                width: 8
                height: parent.height + 10
                anchors.verticalCenter: parent.verticalCenter
                color: indicatorColor
                radius: 4

                // Позиция: offset нормализован к ширине шкалы
                x: (scale.width / 2) + (offset / maxOffset) * (scale.width / 2) - width / 2

                Behavior on x {
                    NumberAnimation { duration: 100 }
                }
            }

            // Центральная метка
            Rectangle {
                anchors.centerIn: parent
                width: 2
                height: parent.height
                color: "white"
            }
        }

        // Значение
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: (offset >= 0 ? "+" : "") + offset.toFixed(2) + " м"
            color: indicatorColor
            font.pixelSize: 20
            font.bold: true
        }

        // Направление
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: {
                if (Math.abs(offset) < 0.05) return "В центре"
                return offset > 0 ? "→ Вправо" : "← Влево"
            }
            color: "#aaa"
            font.pixelSize: 12
        }
    }
}
```

---

## WarningPanel.qml

**Назначение:** Панель отображения предупреждений.

### Структура

```qml
import QtQuick

Rectangle {
    id: root

    property var model
    property var config

    width: config ? config.warningPanelWidth : 360
    height: warningList.height + 20
    color: "#CC1a1a2e"
    radius: 10
    visible: model && model.count > 0

    Column {
        id: warningList
        anchors.centerIn: parent
        width: parent.width - 20
        spacing: 8

        Repeater {
            model: root.model

            delegate: WarningItem {
                width: warningList.width
                warningType: model.type
                severity: model.severity
                message: model.message
                distance: model.distance
                iconSource: model.icon
            }
        }
    }
}
```

### WarningItem (вложенный компонент)

```qml
Rectangle {
    id: warningItem

    property int warningType
    property int severity       // 0=Info, 1=Warning, 2=Critical
    property string message
    property real distance
    property string iconSource

    height: 60
    radius: 8

    // Цвет фона по severity
    color: {
        switch (severity) {
            case 0: return "#2196F3"   // Info - синий
            case 1: return "#FF9800"   // Warning - оранжевый
            case 2: return "#F44336"   // Critical - красный
            default: return "#666"
        }
    }

    // Анимация пульсации для Critical
    SequentialAnimation on opacity {
        running: severity === 2
        loops: Animation.Infinite
        NumberAnimation { to: 0.7; duration: 300 }
        NumberAnimation { to: 1.0; duration: 300 }
    }

    Row {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Иконка
        Image {
            source: iconSource
            width: 40
            height: 40
            anchors.verticalCenter: parent.verticalCenter
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 4

            Text {
                text: message
                color: "white"
                font.pixelSize: 14
                font.bold: true
            }

            Text {
                visible: distance > 0
                text: "Расстояние: " + distance.toFixed(0) + " м"
                color: "#ddd"
                font.pixelSize: 12
            }
        }
    }
}
```

---

## CameraController.qml

**Назначение:** Интерактивное управление камерой (опционально).

### Структура

```qml
import QtQuick
import QtQuick3D

Node {
    id: root

    property PerspectiveCamera camera
    property real rotationSpeed: 0.5
    property real zoomSpeed: 50

    // Текущие углы камеры
    property real pitch: -20
    property real yaw: 153
    property real distance: 2000

    // Обновление позиции камеры
    function updateCamera() {
        if (!camera) return

        var pitchRad = pitch * Math.PI / 180
        var yawRad = yaw * Math.PI / 180

        camera.position = Qt.vector3d(
            distance * Math.cos(pitchRad) * Math.sin(yawRad),
            distance * Math.sin(-pitchRad),
            distance * Math.cos(pitchRad) * Math.cos(yawRad)
        )

        camera.eulerRotation = Qt.vector3d(pitch, yaw, 0)
    }

    // Обработка мыши
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        property point lastPos

        onPressed: lastPos = Qt.point(mouse.x, mouse.y)

        onPositionChanged: {
            if (mouse.buttons & Qt.LeftButton) {
                // Вращение
                yaw += (mouse.x - lastPos.x) * rotationSpeed
                pitch += (mouse.y - lastPos.y) * rotationSpeed
                pitch = Math.max(-89, Math.min(89, pitch))
                updateCamera()
            }
            lastPos = Qt.point(mouse.x, mouse.y)
        }

        onWheel: {
            // Зум
            distance -= wheel.angleDelta.y / 120 * zoomSpeed
            distance = Math.max(500, Math.min(5000, distance))
            updateCamera()
        }
    }

    Component.onCompleted: updateCamera()
}
```

---

## Theme.qml

**Назначение:** Централизованное управление цветами и стилями.

### Структура

```qml
pragma Singleton
import QtQuick

QtObject {
    // Основные цвета
    readonly property color background: "#1a1a2e"
    readonly property color surface: "#16213e"
    readonly property color primary: "#4a90d9"
    readonly property color accent: "#e94560"

    // Цвета разметки
    readonly property color lineWhite: "#ffffff"
    readonly property color lineYellow: "#FFD700"
    readonly property color lineRed: "#FF4444"

    // Цвета предупреждений
    readonly property color warningInfo: "#2196F3"
    readonly property color warningWarn: "#FF9800"
    readonly property color warningCritical: "#F44336"

    // Цвета индикатора смещения
    readonly property color offsetSafe: "#4CAF50"
    readonly property color offsetWarning: "#FF9800"
    readonly property color offsetCritical: "#F44336"

    // Шрифты
    readonly property int fontSizeSmall: 12
    readonly property int fontSizeNormal: 14
    readonly property int fontSizeLarge: 18
    readonly property int fontSizeTitle: 24

    // Отступы
    readonly property int spacingSmall: 4
    readonly property int spacingNormal: 8
    readonly property int spacingLarge: 16

    // Скругления
    readonly property int radiusSmall: 4
    readonly property int radiusNormal: 8
    readonly property int radiusLarge: 12
}
```

### Использование темы

```qml
import "Theme"

Rectangle {
    color: Theme.background

    Text {
        color: Theme.primary
        font.pixelSize: Theme.fontSizeNormal
    }
}
```

---

## Интеграция с C++

### Регистрация ViewModel в QML

```cpp
// В main.cpp или AppController
qmlRegisterUncreatableType<LaneStateViewModel>(
    "Dashboard", 1, 0, "LaneStateViewModel",
    "Cannot create LaneStateViewModel from QML"
);

// Передача ViewModel в QML
QQmlApplicationEngine engine;
engine.rootContext()->setContextProperty("laneViewModel", laneViewModel);
engine.rootContext()->setContextProperty("markingListModel", markingListModel);
engine.rootContext()->setContextProperty("warningListModel", warningListModel);
engine.rootContext()->setContextProperty("sceneConfig", sceneConfig);
```

### Использование в QML

```qml
import QtQuick

Item {
    // Привязка к данным из C++
    Text {
        text: "Смещение: " + (laneViewModel ? laneViewModel.centerOffset.toFixed(2) : "N/A") + " м"
        visible: laneViewModel && laneViewModel.valid
    }

    // Список объектов
    ListView {
        model: markingListModel

        delegate: Text {
            text: model.className + " @ " + model.yMeters.toFixed(1) + "м"
        }
    }
}
```
