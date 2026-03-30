# API Reference

## Обзор

Документация публичных API классов приложения Dashboard for Road Marking.

---

## AppController

**Файл:** `app/AppController.hpp`

Центральный контроллер приложения. Координирует все компоненты и предоставляет доступ к ViewModel'ам.

### Свойства (Q_PROPERTY)

| Свойство | Тип | Доступ | Описание |
|----------|-----|--------|----------|
| `laneViewModel` | `LaneStateViewModel*` | READ | ViewModel состояния полос |
| `markingListModel` | `MarkingObjectListModel*` | READ | Модель списка объектов разметки |
| `warningListModel` | `WarningListModel*` | READ | Модель списка предупреждений |
| `fittedLineListModel` | `FittedLineListModel*` | READ | Модель полиномиальных линий |
| `isFullyConnected` | `bool` | READ | Статус полного подключения |

### Методы

#### initialize

```cpp
bool initialize(const QString& configPath, const QString& sceneConfigPath = QString())
```

Инициализирует все компоненты приложения.

**Параметры:**
- `configPath` — путь к файлу config.json
- `sceneConfigPath` — путь к файлу scene_config.json (опционально)

**Возвращает:** `true` при успешной инициализации

**Пример:**
```cpp
AppController controller;
if (!controller.initialize("config.json", "scene_config.json")) {
    qCritical() << "Failed to initialize";
    return 1;
}
```

#### shutdown

```cpp
void shutdown()
```

Корректно завершает работу всех компонентов.

#### connectionManager

```cpp
ConnectionManager* connectionManager() const
```

**Возвращает:** указатель на менеджер TCP соединения

#### videoWidget

```cpp
NetworkVideoWidget* videoWidget() const
```

**Возвращает:** указатель на виджет видео

### Сигналы

| Сигнал | Параметры | Описание |
|--------|-----------|----------|
| `connectionStateChanged` | — | Изменилось состояние соединения |
| `initialized` | — | Инициализация завершена |
| `shutdownComplete` | — | Завершение работы завершено |

---

## ConnectionManager

**Файл:** `network/ConnectionManager.h`

Управление TCP соединением для получения данных разметки.

### Перечисления

#### State

```cpp
enum class State {
    Disconnected,    // Отключён
    Connecting,      // Подключение
    Connected,       // Подключён
    Disconnecting,   // Отключение
    Reconnecting,    // Переподключение
    Error           // Ошибка
};
```

### Свойства (Q_PROPERTY)

| Свойство | Тип | Доступ | Описание |
|----------|-----|--------|----------|
| `connected` | `bool` | READ | Статус подключения |
| `state` | `State` | READ | Текущее состояние |
| `lastError` | `QString` | READ | Последняя ошибка |

### Методы

#### connectToHost

```cpp
Q_INVOKABLE void connectToHost(const QString& host, int port)
```

Подключается к TCP серверу.

**Параметры:**
- `host` — IP адрес или hostname сервера
- `port` — порт сервера

**Пример (QML):**
```qml
connectionManager.connectToHost("192.168.1.100", 9000)
```

#### disconnectFromHost

```cpp
Q_INVOKABLE void disconnectFromHost()
```

Отключается от сервера.

#### setAutoReconnect

```cpp
void setAutoReconnect(bool enabled)
```

Включает/выключает автоматическое переподключение.

#### setReconnectInterval

```cpp
void setReconnectInterval(int ms)
```

Устанавливает интервал переподключения в миллисекундах.

#### laneState

```cpp
const domain::LaneState& laneState() const
```

**Возвращает:** текущее состояние полос движения

#### markingModel

```cpp
const domain::MarkingObjectModel& markingModel() const
```

**Возвращает:** модель объектов разметки

### Сигналы

| Сигнал | Параметры | Описание |
|--------|-----------|----------|
| `connectedChanged` | `bool connected` | Изменился статус подключения |
| `stateChanged` | `State state` | Изменилось состояние |
| `errorOccurred` | `QString error` | Произошла ошибка |
| `laneStateUpdated` | — | Обновлено состояние полос |
| `markingModelUpdated` | — | Обновлена модель разметки |
| `warningModelUpdated` | — | Обновлены предупреждения |
| `laneLinesReceived` | `LaneLines lines` | Получены данные линий |
| `roadObjectsReceived` | `RoadObjects objects` | Получены данные объектов |

---

## LaneStateViewModel

**Файл:** `viewmodels/LaneStateViewModel.h`

ViewModel для отображения состояния полос движения в QML.

### Свойства (Q_PROPERTY)

#### Общие

| Свойство | Тип | Описание |
|----------|-----|----------|
| `valid` | `bool` | Данные валидны |
| `centerOffset` | `float` | Смещение от центра полосы (м) |
| `quality` | `int` | Качество детекции (0-100) |
| `timestampMs` | `quint64` | Временная метка |

#### Левая линия

| Свойство | Тип | Описание |
|----------|-----|----------|
| `laneTypeLeft` | `int` | Тип линии (0-3) |
| `laneColorLeft` | `int` | Цвет линии (0-3) |
| `leftOffset` | `float` | Смещение линии (м) |
| `leftConfidence` | `float` | Уверенность (0-100) |

#### Правая линия

| Свойство | Тип | Описание |
|----------|-----|----------|
| `laneTypeRight` | `int` | Тип линии (0-3) |
| `laneColorRight` | `int` | Цвет линии (0-3) |
| `rightOffset` | `float` | Смещение линии (м) |
| `rightConfidence` | `float` | Уверенность (0-100) |

### Методы

#### updateFromDomain

```cpp
void updateFromDomain(const domain::LaneState& state)
```

Обновляет ViewModel из доменной модели.

### Сигналы

Для каждого Q_PROPERTY есть соответствующий сигнал `<property>Changed()`.

### Использование в QML

```qml
Text {
    text: "Смещение: " + laneViewModel.centerOffset.toFixed(2) + " м"
    color: Math.abs(laneViewModel.centerOffset) > 0.3 ? "red" : "green"
    visible: laneViewModel.valid
}
```

---

## MarkingObjectListModel

**Файл:** `viewmodels/MarkingObjectListModel.h`

Модель списка объектов разметки для ListView в QML.

### Роли (Roles)

| Роль | Тип | Описание |
|------|-----|----------|
| `ClassIdRole` | `int` | ID класса объекта |
| `ClassNameRole` | `QString` | Название класса |
| `XMetersRole` | `float` | Позиция X (м) |
| `YMetersRole` | `float` | Позиция Y (м) |
| `WidthMetersRole` | `float` | Ширина (м) |
| `HeightMetersRole` | `float` | Высота (м) |
| `ConfidenceRole` | `float` | Уверенность (0-100) |
| `IsCrosswalkRole` | `bool` | Это пешеходный переход |
| `IsArrowRole` | `bool` | Это стрелка |
| `CenterXPixelsRole` | `int` | Центр X (px) |
| `CenterYPixelsRole` | `int` | Центр Y (px) |
| `WidthPixelsRole` | `int` | Ширина (px) |
| `HeightPixelsRole` | `int` | Высота (px) |

### Свойства (Q_PROPERTY)

| Свойство | Тип | Описание |
|----------|-----|----------|
| `count` | `int` | Количество объектов |

### Методы

#### updateFromDomain

```cpp
void updateFromDomain(const domain::MarkingObjectModel& model)
```

Обновляет модель из доменной модели.

#### rowCount

```cpp
int rowCount(const QModelIndex& parent = QModelIndex()) const override
```

**Возвращает:** количество элементов

#### data

```cpp
QVariant data(const QModelIndex& index, int role) const override
```

**Возвращает:** данные для указанной роли

### Использование в QML

```qml
ListView {
    model: markingListModel

    delegate: Rectangle {
        width: parent.width
        height: 50

        Row {
            spacing: 10
            Text { text: model.className }
            Text { text: model.yMeters.toFixed(1) + " м" }
            Text {
                text: model.confidence.toFixed(0) + "%"
                color: model.confidence > 80 ? "green" : "orange"
            }
        }
    }
}
```

---

## WarningListModel

**Файл:** `viewmodels/WarningListModel.h`

Модель списка предупреждений для QML.

### Роли (Roles)

| Роль | Тип | Описание |
|------|-----|----------|
| `TypeRole` | `int` | Тип предупреждения |
| `SeverityRole` | `int` | Серьёзность (0-2) |
| `MessageRole` | `QString` | Текст сообщения |
| `DistanceRole` | `float` | Расстояние (м) |
| `IconRole` | `QString` | Путь к иконке |
| `ColorRole` | `QColor` | Цвет предупреждения |

### Свойства (Q_PROPERTY)

| Свойство | Тип | Описание |
|----------|-----|----------|
| `count` | `int` | Количество предупреждений |
| `hasWarnings` | `bool` | Есть ли предупреждения |

### Методы

#### updateWarnings

```cpp
void updateWarnings(const std::vector<domain::Warning>& warnings)
```

Обновляет список предупреждений.

### Использование в QML

```qml
Repeater {
    model: warningListModel

    delegate: Rectangle {
        color: model.color
        opacity: model.severity === 2 ? 0.9 : 0.7

        Text {
            text: model.message
            color: "white"
        }

        Text {
            text: model.distance > 0 ? model.distance.toFixed(0) + " м" : ""
        }
    }
}
```

---

## FittedLineListModel

**Файл:** `viewmodels/FittedLineListModel.h`

Модель списка полиномиальных линий для QML.

### Роли (Roles)

| Роль | Тип | Описание |
|------|-----|----------|
| `LineIdRole` | `int` | ID линии |
| `PolyARole` | `float` | Коэффициент A (y²) |
| `PolyBRole` | `float` | Коэффициент B (y) |
| `PolyCRole` | `float` | Коэффициент C |
| `ConfidenceRole` | `float` | Уверенность |

### Свойства (Q_PROPERTY)

| Свойство | Тип | Описание |
|----------|-----|----------|
| `count` | `int` | Количество линий |

### Использование в QML

```qml
Repeater3D {
    model: fittedLineListModel

    delegate: PolyLine3D {
        polyA: model.polyA
        polyB: model.polyB
        polyC: model.polyC
        confidence: model.confidence
    }
}
```

---

## SceneConfigViewModel

**Файл:** `viewmodels/SceneConfigViewModel.hpp`

ViewModel конфигурации 3D сцены для QML.

### Свойства (Q_PROPERTY)

#### Масштаб

| Свойство | Тип | Описание |
|----------|-----|----------|
| `scaleFactor` | `int` | Масштаб сцены |

#### Камера

| Свойство | Тип | Описание |
|----------|-----|----------|
| `cameraHeight` | `float` | Высота камеры |
| `cameraDistance` | `float` | Расстояние камеры |
| `cameraPitchAngle` | `float` | Угол наклона |
| `clipNear` | `float` | Ближняя плоскость |
| `clipFar` | `float` | Дальняя плоскость |

#### Освещение

| Свойство | Тип | Описание |
|----------|-----|----------|
| `mainLightBrightness` | `float` | Яркость основного света |
| `secondaryLightBrightness` | `float` | Яркость вторичного света |

#### Дорога

| Свойство | Тип | Описание |
|----------|-----|----------|
| `roadWidthM` | `float` | Ширина дороги (м) |
| `roadLengthM` | `float` | Длина дороги (м) |
| `edgeLineWidth` | `float` | Ширина краевой линии |
| `centerLineWidth` | `float` | Ширина центральной линии |

#### Разметка

| Свойство | Тип | Описание |
|----------|-----|----------|
| `markingYPosition` | `float` | Высота разметки над дорогой |
| `markingHeight` | `float` | Толщина разметки |
| `minOpacity` | `float` | Минимальная прозрачность |
| `maxOpacity` | `float` | Максимальная прозрачность |
| `confidenceDivisor` | `float` | Делитель confidence |

#### Индикатор смещения

| Свойство | Тип | Описание |
|----------|-----|----------|
| `centerOffsetWidth` | `int` | Ширина индикатора |
| `centerOffsetHeight` | `int` | Высота индикатора |
| `maxOffsetM` | `float` | Максимальное смещение |
| `safeThresholdM` | `float` | Безопасный порог |
| `criticalThresholdM` | `float` | Критический порог |

---

## NetworkVideoWidget

**Файл:** `videowidget/widgets/NetworkVideoWidget.hpp`

Виджет для отображения сетевого видеопотока с оверлеем.

### Свойства (Q_PROPERTY)

| Свойство | Тип | Описание |
|----------|-----|----------|
| `connected` | `bool` | Статус подключения |

### Методы

#### connectToSource

```cpp
Q_INVOKABLE void connectToSource(const QString& url)
```

Подключается к видеопотоку.

**Поддерживаемые URL:**
- `udp://host:port` — UDP multicast
- `rtsp://host:port/path` — RTSP поток
- `rtp://host:port` — RTP поток
- `file:///path` — локальный файл
- `videotestsrc` — тестовый источник

#### disconnect

```cpp
Q_INVOKABLE void disconnect()
```

Отключается от видеопотока.

#### setFrameProcessor

```cpp
void setFrameProcessor(IVideoFrameProcessor* processor)
```

Устанавливает процессор кадров для оверлея.

### Сигналы

| Сигнал | Параметры | Описание |
|--------|-----------|----------|
| `connectedChanged` | `bool connected` | Изменился статус |
| `connectionFailed` | `QString error` | Ошибка подключения |
| `frameDisplayed` | `quint64 timestamp` | Отображён кадр |

---

## MarkingOverlayProcessor

**Файл:** `videowidget/processors/MarkingOverlayProcessor.hpp`

Процессор для наложения разметки на видеокадры.

### Методы

#### setLaneStateViewModel

```cpp
void setLaneStateViewModel(LaneStateViewModel* vm)
```

Устанавливает источник данных о полосах.

#### setMarkingObjectListModel

```cpp
void setMarkingObjectListModel(MarkingObjectListModel* model)
```

Устанавливает источник данных об объектах.

#### setFittedLineListModel

```cpp
void setFittedLineListModel(FittedLineListModel* model)
```

Устанавливает источник полиномиальных линий.

#### updateMarkings

```cpp
void updateMarkings(const domain::MarkingObjectModel& model)
```

Обновляет данные об объектах разметки.

#### updateFittedLines

```cpp
void updateFittedLines(const domain::FittedLinesModel& model)
```

Обновляет данные о полиномиальных линиях.

#### setSourceSize

```cpp
void setSourceSize(int width, int height)
```

Устанавливает размер исходного изображения для преобразования координат.

#### processFrame

```cpp
void processFrame(QImage& frame) override
```

Обрабатывает кадр, добавляя оверлей.

---

## ProtoParser

**Файл:** `parser/proto_parser.h`

Парсер бинарного TCP протокола V2.

### Интерфейс IMessageHandler

```cpp
class IMessageHandler {
public:
    virtual ~IMessageHandler() = default;
    virtual void onLaneLines(const laneproto::LaneLines& msg) = 0;
    virtual void onRoadObjects(const laneproto::RoadObjects& msg) = 0;
};
```

### Методы

#### setHandler

```cpp
void setHandler(IMessageHandler* handler)
```

Устанавливает обработчик сообщений.

#### feed

```cpp
void feed(const uint8_t* data, size_t size)
void feed(const QByteArray& data)
```

Подаёт данные на вход парсера.

#### reset

```cpp
void reset()
```

Сбрасывает состояние парсера.

#### framesReceived

```cpp
uint64_t framesReceived() const
```

**Возвращает:** количество успешно принятых кадров

#### framesDropped

```cpp
uint64_t framesDropped() const
```

**Возвращает:** количество отброшенных кадров

---

## Доменные модели

### LaneState

**Файл:** `domain/LaneState.h`

```cpp
namespace domain {

class LaneState {
public:
    enum class LineType { Unknown = 0, Solid = 1, Dashed = 2, Double = 3 };
    enum class LineColor { Unknown = 0, White = 1, Yellow = 2, Red = 3 };

    struct LineInfo {
        LineType type = LineType::Unknown;
        LineColor color = LineColor::Unknown;
        float offset_m = 0.0f;
        float confidence = 0.0f;
    };

    LineInfo left_line;
    LineInfo right_line;
    float center_offset_m = 0.0f;
    int quality = 0;
    uint64_t timestamp_ms = 0;

    bool isValid() const;
};

} // namespace domain
```

### MarkingObject

**Файл:** `domain/MarkingObject.h`

```cpp
namespace domain {

class MarkingObject {
public:
    enum class ClassId {
        Unknown = 0x00,
        Crosswalk = 0x02,
        StopLine = 0x05,
        ArrowLeft = 0x0B,
        ArrowStraight = 0x0C,
        ArrowRight = 0x0D,
        ArrowLeftRight = 0x0E,
        ArrowStraightLeft = 0x0F,
        ArrowStraightRight = 0x10,
        ArrowUturn = 0x11
    };

    ClassId class_id = ClassId::Unknown;
    float x_meters = 0.0f;
    float y_meters = 0.0f;
    float width_meters = 0.0f;
    float height_meters = 0.0f;
    int center_x_pixels = 0;
    int center_y_pixels = 0;
    int width_pixels = 0;
    int height_pixels = 0;
    float confidence = 0.0f;
    uint64_t timestamp_ms = 0;

    bool isCrosswalk() const;
    bool isArrow() const;
    bool isStopLine() const;
    QString className() const;
};

class MarkingObjectModel {
public:
    void addObject(const MarkingObject& obj);
    void clear();
    const std::vector<MarkingObject>& objects() const;
    std::optional<MarkingObject> nearestCrosswalk() const;
};

} // namespace domain
```

### Warning

**Файл:** `domain/Warning.h`

```cpp
namespace domain {

class Warning {
public:
    enum class Type {
        None,
        CrosswalkAhead,
        LaneDepartureLeft,
        LaneDepartureRight,
        SolidLineCrossing
    };

    enum class Severity { Info, Warning, Critical };

    Type type = Type::None;
    Severity severity = Severity::Info;
    QString message;
    float distance_m = 0.0f;
    uint64_t timestamp_ms = 0;

    QString icon() const;
    QColor color() const;
};

} // namespace domain
```
