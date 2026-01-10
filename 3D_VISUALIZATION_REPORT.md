# Детальный отчет: 3D Визуализация дорожной разметки Dashboard-for-road-marking

## 1. СТРУКТУРА ПРОЕКТА

### Организация директорий:
```
Dashboard-for-road-marking/
├── app/                      # Контроллер приложения
├── config/                   # Конфигурация (JSON, С++)
├── domain/                   # Бизнес-логика (MarkingObject, LaneState, etc)
├── network/                  # TCP соединение и обработка сообщений
├── parser/                   # Парсинг protobuf сообщений
├── logger/                   # Логирование
├── ui/
│   ├── DashboardWidget.cpp   # Main QQuickWidget контейнер
│   └── qml/                  # QML файлы для 3D сцены
│       ├── Dashboard.qml     # Root QML файл
│       ├── CarScene.qml      # 3D сцена с компонентами
│       ├── RoadPlane.qml     # Дорожное покрытие и разметки
│       ├── MarkingLine.qml   # Визуализация отдельной разметки
│       ├── CarModel.qml      # 3D модель автомобиля
│       ├── CameraController.qml    # Управление камерой (WASD)
│       ├── CenterOffsetIndicator.qml  # UI индикатор смещения
│       ├── WarningPanel.qml  # Панель предупреждений
│       └── ConnectionStatus.qml
├── viewmodels/               # ViewModel для QML
│       ├── MarkingObjectListModel.h/.cpp
│       ├── LaneStateViewModel.h/.cpp
│       ├── SceneConfigViewModel.hpp/.cpp
│       └── WarningListModel.h/.cpp
├── videowidget/              # Видео-поток и наложение разметок
├── resources/
│   └── resources.qrc         # QML ресурсы
└── CMakeLists.txt            # Build система (CMake)
```

### Build система: CMake (Qt6)
- C++ Standard: C++17
- Qt6 Components: Core, Gui, Widgets, Network, Multimedia, Qml, Quick, **Quick3D**, QuickWidgets
- Graphics Backend: OpenGL 3.3 Core Profile


## 2. QML ФАЙЛЫ И 3D КОМПОНЕНТЫ

### Используемые компоненты Qt3D:
1. **QtQuick3D** (import QtQuick3D)
2. **View3D** - Контейнер для 3D сцены
3. **SceneEnvironment** - Параметры окружения (фон, антиалиасинг)
4. **Node** - Иерархические компоненты сцены
5. **Model** - Примитивы (#Cube, #Sphere)
6. **PrincipledMaterial** - PBR материал
7. **DirectionalLight** - Направленные источники света
8. **PointLight** - Точечный источник света
9. **PerspectiveCamera** - Камера
10. **Repeater3D** - Повторение компонентов в 3D (для разметок и точек)

### CarScene.qml (Главная 3D сцена)

**Функции:**
- Содержит всю 3D визуализацию дорожной разметки
- Использует isometric/перспективный вид

**Компоненты:**
```
CarScene (Node)
├── PerspectiveCamera
│   ├── position: (0, cameraHeight, cameraDistance)
│   ├── euler: (cameraPitchAngle, 0, 0)
│   └── fieldOfView: 75
├── DirectionalLights (3 источника)
│   ├── Main light: 0.7 яркость
│   ├── Secondary light: 0.45 яркость
│   └── Tertiary light: 0.25 яркость
├── PointLight (для выделения машины)
├── RoadPlane (дорожное полотно + разметки)
├── CarModel (3D модель машины)
├── Repeater3D (для MarkingLine элементов)
├── Repeater3D (левые граничные точки полосы)
└── Repeater3D (правые граничные точки полосы)
```

**Важный параметр: scale_factor**
- Преобразует метры в единицы QML сцены
- Значение: 1.0 (по умолчанию из конфига)
- Используется для масштабирования всех геометрических данных

### RoadPlane.qml (Дорожное полотно)

**Структура:**
```
RoadPlane (Node)
├── Model (#Cube) - Поверхность дороги
│   ├── Размер: 12м x 100м x 0.01м (ширина x длина x толщина)
│   ├── Y позиция: -0.05м
│   └── Материал: темно-серый, матовый
├── Edge line левый (белый кубик)
│   ├── Размер: 0.15м (ширина)
│   └── Материал: светло-серый, выпуклый
├── Edge line правый
├── Center dashed line (Repeater3D)
│   ├── Количество штрихов: 30
│   ├── Длина штриха: 50% от интервала
│   └── Материал: светлый, с эмиссией
```

**Координатная система:**
- X ось: слева-справа (lane width)
- Y ось: вверх-вниз
- Z ось: вперед-назад (по направлению движения, инвертирована)
  - Отрицательное Z - впереди машины
  - Положительное Z - позади машины

### MarkingLine.qml (Отдельная разметка объект)

**Входные параметры:**
```qml
required property real xMeters          // Позиция X (метры)
required property real yMeters          // Позиция Y (метры) 
required property real lengthMeters     // Длина разметки
required property real widthMeters      // Ширина разметки
required property real yawDeg           // Угол поворота (градусы)
required property string className      // Тип разметки
required property bool isCrosswalk      // Это зебра?
required property bool isArrow          // Это стрелка?
required property bool isValid          // Валидна ли разметка?
required property int confidence        // Уверенность (0-255)
required property string lineColor      // Цвет ("yellow", "white", "red")
required property string lineStyle      // Стиль ("dashed", "solid")
```

**Визуализация:**
- Model (#Cube) с применением масштабирования
- Position: (x*scaleFactor, markingYPosition, -y*scaleFactor)
  - Инверсия Y для соответствия направлению камеры
- Scale: (width*scaleFactor, markingHeight, length*scaleFactor)
- Rotation: eulerRotation.y = -yawDeg
- Opacity: зависит от confidence (0.3 - 1.0)
- Материал: цветной куб с эмиссией, зависит от lineColor

**Цветовая схема:**
- Yellow (#ffd700) - желтые разметки
- Red (#ff5555) - красные разметки
- White (#ffffff) - белые разметки
- Crosswalk - белый
- Arrow - желтый

**Opacity расчет:**
```
opacity = max(0.3, min(1.0, confidence / 100.0))
```

### CarModel.qml (3D модель машины)

**Реальные размеры:**
- Ширина: 1.8м
- Длина: 4.5м
- Высота: 1.5м

**Структура (стэк кубов):**
1. Shadow footprint (темный, 5% от высоты)
2. Base body (темно-синий выступ, 30% от высоты)
3. Main body (яркий cyan #00e0ff, 55% от высоты)
4. Roof highlight (светлый cyan, 30% от высоты)

**Позиция:** 0, carYElevation*scaleFactor, 0

### Lane boundary points (в CarScene)

**Функция:** Визуализация граничных точек полосы слева/справа

**Левые точки Repeater3D:**
- Модель: сферы (#Sphere)
- Размер: 0.08 * scaleFactor (маркер)
- Материал: золотистый (#ffd700) с эмиссией
- Позиция: (x*scaleFactor, markerYPosition*scaleFactor, -y*scaleFactor)

**Правые точки Repeater3D:**
- Модель: сферы (#Sphere)
- Размер: 0.08 * scaleFactor
- Материал: белый (#ffffff) с эмиссией
- Позиция: (x*scaleFactor, markerYPosition*scaleFactor, -y*scaleFactor)

### Dashboard.qml (Root контейнер)

**Структура:**
```
Item (root)
├── View3D
│   ├── SceneEnvironment (MSAA, #1a1a1a background)
│   └── CarScene (см. выше)
├── CameraController (overlay для управления)
└── UI Overlays (z:10)
    ├── Camera info box (top-right)
    ├── WarningPanel (top-left)
    ├── CenterOffsetIndicator (bottom-left)
    └── ConnectionStatus (bottom-right)
```

### CameraController.qml

**Управление камерой:**
- W/A/S/D - движение (40 единиц)
- Space - вверх
- Shift - вниз
- Arrow keys - ротация (2 градуса за нажатие)
- R - reset позиция

**Default position:** (0, 15, -25)
**Default rotation:** (-35, 0, 0)


## 3. C++/QT ВИД МОДЕЛЬ И ПЕРЕДАЧА ДАННЫХ

### Архитектура передачи данных:

```
TCP Network (TcpReaderWorker)
    ↓ (protobuf сообщения)
ConnectionManager
    ├→ LaneState (domain)
    ├→ MarkingObjectModel (domain)
    ├→ FittedLinesModel (domain)
    └→ WarningModel (domain)
    
    ↓ (сигналы Qt)
    
ViewModel слой
    ├→ LaneStateViewModel (Q_PROPERTY)
    ├→ MarkingObjectListModel (QAbstractListModel)
    ├→ SceneConfigViewModel (Q_PROPERTY)
    └→ WarningListModel (QAbstractListModel)
    
    ↓ (setContextProperty)
    
QML сцена
    ├→ laneViewModel
    ├→ markingModel (список для Repeater3D)
    ├→ warningModel
    └→ sceneConfig
```

### DashboardWidget.cpp (QQuickWidget контейнер)

**Установка контекста QML:**
```cpp
context->setContextProperty("appController", controller);
context->setContextProperty("laneViewModel", controller->laneViewModel());
context->setContextProperty("markingModel", controller->markingListModel());
context->setContextProperty("warningModel", controller->warningListModel());

// SceneConfig из JSON файла
config::SceneConfig sceneConfig = 
    config::SceneConfig::loadFromFile("config/scene_config.json");
auto* sceneConfigViewModel = 
    new viewmodels::SceneConfigViewModel(sceneConfig, this);
context->setContextProperty("sceneConfig", sceneConfigViewModel);

quickWidget_->setSource(QUrl("qrc:/qml/Dashboard.qml"));
```

**OpenGL настройки:**
- Depth buffer: 24-bit
- Stencil buffer: 8-bit
- OpenGL 3.3 Core Profile
- MSAA 4x


### AppController.hpp (Главный контроллер)

**Сигналы и слоты:**
```cpp
Q_PROPERTY(viewmodels::LaneStateViewModel* laneViewModel 
           READ laneViewModel CONSTANT)
Q_PROPERTY(viewmodels::MarkingObjectListModel* markingListModel 
           READ markingListModel CONSTANT)
Q_PROPERTY(viewmodels::WarningListModel* warningListModel 
           READ warningListModel CONSTANT)

signals:
    void initializationComplete();
    void dataConnectionChanged(bool connected);
    void videoConnectionChanged(bool connected);
    void connectionStateChanged(bool connected);
    void statusMessageChanged(const QString& message);
    void criticalError(const QString& message);
```

**Основной поток:**
1. `initialize()` - загрузка конфига
2. `createComponents()` - создание ConnectionManager, VideoWidget
3. `configureComponents()` - настройка сетевых параметров
4. `wireComponents()` - подключение сигналов


### ConnectionManager.h (Получение данных по TCP)

**Q_PROPERTY:**
```cpp
Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
Q_PROPERTY(State state READ state NOTIFY stateChanged)
Q_PROPERTY(viewmodels::LaneStateViewModel* laneViewModel 
           READ laneViewModel CONSTANT)
Q_PROPERTY(viewmodels::MarkingObjectListModel* markingListModel 
           READ markingListModel CONSTANT)
```

**Сигналы обновления:**
```cpp
signals:
    void laneStateUpdated();           // Граница полосы изменилась
    void markingModelUpdated();        // Разметки обновились
    void fittedLinesModelUpdated();    // Fitted lines обновились
    void warningModelUpdated();        // Предупреждения обновились
```

**Приватные методы обработки:**
```cpp
void laneSummaryReceived(const laneproto::LaneSummary& summary);
void markingObjectsReceived(const laneproto::MarkingObjects& objects);
void laneDetailsReceived(const laneproto::LaneDetails& details);
void markingObjectsExReceived(const laneproto::MarkingObjects& objects);
void fittedLinesReceived(const laneproto::FittedLines& lines);
void updateWarnings(std::uint64_t timestamp_ms);
```

### LaneStateViewModel.h (Граница полосы)

**Q_PROPERTY (все CONSTANT, автоматически notify через сигналы):**
```cpp
Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)
Q_PROPERTY(QString laneTypeLeft READ laneTypeLeft NOTIFY laneTypeLeftChanged)
Q_PROPERTY(QString laneTypeRight READ laneTypeRight NOTIFY laneTypeRightChanged)
Q_PROPERTY(float leftOffsetMeters READ leftOffsetMeters NOTIFY leftOffsetChanged)
Q_PROPERTY(float rightOffsetMeters READ rightOffsetMeters NOTIFY rightOffsetChanged)
Q_PROPERTY(float laneWidthMeters READ laneWidthMeters NOTIFY laneWidthChanged)
Q_PROPERTY(float centerOffsetMeters READ centerOffsetMeters NOTIFY centerOffsetChanged)
Q_PROPERTY(int qualityPercent READ qualityPercent NOTIFY qualityChanged)
Q_PROPERTY(bool isQualityGood READ isQualityGood NOTIFY qualityChanged)
Q_PROPERTY(quint64 timestampMs READ timestampMs NOTIFY timestampChanged)
Q_PROPERTY(bool hasDetails READ hasDetails NOTIFY detailsChanged)

// Детали по каждой границе
Q_PROPERTY(QString laneColorLeft READ laneColorLeft NOTIFY laneColorLeftChanged)
Q_PROPERTY(QString laneColorRight READ laneColorRight NOTIFY laneColorRightChanged)
Q_PROPERTY(float laneWidthLeftMeters READ laneWidthLeftMeters NOTIFY laneWidthLeftChanged)
Q_PROPERTY(float laneWidthRightMeters READ laneWidthRightMeters NOTIFY laneWidthRightChanged)
Q_PROPERTY(int laneQualityLeftPercent READ laneQualityLeftPercent NOTIFY laneQualityLeftChanged)
Q_PROPERTY(int laneQualityRightPercent READ laneQualityRightPercent NOTIFY laneQualityRightChanged)

// Граничные ТОЧКИ для визуализации
Q_PROPERTY(QVariantList leftPoints READ leftPoints NOTIFY lanePointsChanged)
Q_PROPERTY(QVariantList rightPoints READ rightPoints NOTIFY lanePointsChanged)
```

**Использование в QML:**
```qml
property bool hasData: laneViewModel && laneViewModel.valid
property real offsetMeters: hasData ? laneViewModel.centerOffsetMeters : 0.0

// В CarScene для граничных сфер:
model: (typeof laneViewModel !== 'undefined' && laneViewModel && 
        laneViewModel.leftPoints) ? laneViewModel.leftPoints : []
```

### MarkingObjectListModel.h (Список разметок)

**Роли для QML доступа:**
```cpp
enum Roles {
    ClassIdRole = Qt::UserRole + 1,
    ClassNameRole,
    XMetersRole,
    YMetersRole,
    LengthMetersRole,
    WidthMetersRole,
    YawDegRole,
    ConfidenceRole,
    IsCrosswalkRole,
    IsArrowRole,
    IsValidRole,
    AreaRole,
    DistanceRole,
    LineColorRole,
    LineStyleRole
};

Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
Q_PROPERTY(quint64 timestampMs READ timestampMs NOTIFY timestampChanged)
```

**Использование в QML:**
```qml
Repeater3D {
    model: markingModel  // QAbstractListModel

    delegate: MarkingLine {
        xMeters: model.xMeters          // из XMetersRole
        yMeters: model.yMeters          // из YMetersRole
        lengthMeters: model.lengthMeters
        widthMeters: model.widthMeters
        yawDeg: model.yawDeg
        className: model.className
        isCrosswalk: model.isCrosswalk
        isArrow: model.isArrow
        isValid: model.isValid
        confidence: model.confidence
        lineColor: model.lineColor
        lineStyle: model.lineStyle
    }
}
```


## 4. PROTOBUF СООБЩЕНИЯ И ТИПЫ ДАННЫХ

### Структура сообщений (laneproto namespace):

#### 4.1 LaneSummary (0x01)
```cpp
struct LaneSummary {
    TimestampMs timestamp_ms;          // uint32
    SequenceNumber seq;                // uint8
    float left_offset_m;               // смещение левой границы
    float right_offset_m;              // смещение правой границы
    LaneType lane_type_left;           // тип линии (Solid/Dashed/Double)
    LaneType lane_type_right;
    uint8_t allowed_maneuvers;         // битовая маска маневров
    uint8_t quality;                   // 0-100 (или raw value)
};
```

#### 4.2 MarkingObjects (0x02 или 0x04 ExMarking)
```cpp
struct MarkingObject {
    MarkingClassId class_id;           // тип объекта (см. ниже)
    float x_m;                         // позиция в метрах
    float y_m;                         // (камера видит +Y вперед)
    float length_m;                    // размеры
    float width_m;
    float yaw_deg;                     // угол ориентации
    uint8_t confidence;                // 0-255
    uint8_t flags;                     // битовые флаги
    LineColor line_color;              // White/Yellow/Red
    LineStyle line_style;              // Solid/Dashed/Double
};

struct MarkingObjects {
    TimestampMs timestamp_ms;
    SequenceNumber seq;
    vector<MarkingObject> objects;
};
```

#### 4.3 LaneDetails (0x03)
```cpp
struct LaneBoundaryDetails {
    LaneType type;
    LineColor color;
    float width_m;                     // ширина линии
    uint8_t quality;
    uint8_t points_count;              // 0-3 точек
    array<LanePoint, 3> points;        // граничные точки
};

struct LaneDetails {
    TimestampMs timestamp_ms;
    SequenceNumber seq;
    float left_offset_m;
    float right_offset_m;
    uint8_t quality;
    LaneBoundaryDetails left;          // детали левой границы
    LaneBoundaryDetails right;         // детали правой границы
};
```

#### 4.4 FittedLines (0x05)
```cpp
struct FittedLine {
    MarkingClassId class_id;
    LineSide side;                     // Left/Right/Center
    LineColor color;
    LineStyle style;
    float poly_a, poly_b, poly_c;      // полином (x = a*y^2 + b*y + c)
    int16_t y_start, y_end;            // диапазон Y координат
    uint8_t confidence;
    uint8_t quality;
};
```

### Перечисления:

**MarkingClassId:**
- Unknown (0x00)
- BoxJunction, Crosswalk (зебра), StopLine
- SolidSingle{White/Yellow/Red}
- Double{White/Yellow}
- Dashed{White/Yellow}
- Arrow{Left/Straight/Right/LeftStraight/RightStraight}
- ChannelizingLine
- {Motor/Bike}Icon

**LineColor:**
- Unknown, White (#ffffff), Yellow (#ffd700), Red (#ff5555)

**LineStyle:**
- Unknown, Solid, Dashed, Double

**LaneType:**
- Unknown, Solid, Dashed, DoubleSolid, DoubleDashed, SolidDashed


## 5. КОНФИГУРАЦИЯ 3D СЦЕНЫ

### Файл: config/scene_config.json

```json
{
  "scale": {
    "scale_factor": 1                  // метры → QML единицы
  },
  
  "camera": {
    "height": 800.0,                   // Y позиция камеры
    "distance": 600.0,                 // Z позиция (отрицательное = впереди)
    "pitch_angle_deg": -30.0,          // угол наклона вниз
    "clip_near": 1.1,                  // ближняя плоскость отсечения
    "clip_far": 100000.0               // дальняя плоскость
  },
  
  "lighting": {
    "main_light_brightness": 0.7,      // основной свет
    "secondary_light_brightness": 0.45,
    "tertiary_light_brightness": 0.25,
    "point_light_y_position": 5.0,     // позиция точечного света
    "point_light_brightness": 8.0,     // интенсивность
    "point_light_constant_fade": 1.0,  // затухание света
    "point_light_linear_fade": 0.05,
    "point_light_quadratic_fade": 0.01
  },
  
  "road": {
    "width_m": 12.0,                   // ширина дороги
    "length_m": 100.0,                 // длина видимой дороги
    "thickness_m": 0.01,               // толщина полотна
    "y_position": -0.05,               // Y позиция дороги
    "edge_line_width": 0.15,           // ширина боковых линий
    "center_line_width": 0.1,          // ширина центральной линии
    "center_dash_count": 30,           // количество штрихов
    "dash_length_ratio": 0.5,          // длина штриха / интервал
    "line_y_offset": 0.02,             // смещение линий выше дороги
    "material": {
      "metalness": 0.0,
      "roughness": 0.9,                // матовость дороги
      "specular": 0.0,
      "emissive": 0.07                 // слабая самосветимость
    }
  },
  
  "marking": {
    "y_position_above_plane": 0.02,    // высота разметок над дорогой
    "min_width_m": 0.05,               // минимальная ширина объекта
    "height_m": 0.03,                  // высота разметки (Y толщина куба)
    "min_length_m": 0.2,               // минимальная длина
    "min_opacity": 0.3,                // минимальная прозрачность
    "max_opacity": 1.0,
    "confidence_divisor": 100.0,       // делитель для расчета opacity
    "emissive_factor_scale": 0.8,      // масштаб самосветимости
    "roughness_solid": 0.55,
    "roughness_dashed": 0.8            // более матовые пунктирные
  },
  
  "car": {
    "y_elevation": 0.75,               // высота машины над дорогой
    "marker_scale": 0.08,              // размер маркеров границ
    "marker_y_position": 0.1           // высота маркеров
  },
  
  "center_offset": {
    "width": 320,                      // размер UI компонента
    "height": 130,
    "max_offset_m": 1.0,               // максимальное смещение
    "normalization_divisor": 1.0,      // нормализация позиции
    "normalization_multiplier": 2.0,
    "safe_threshold_m": 0.3,           // зеленая зона
    "critical_threshold_m": 0.6        // желто-красная зона
  },
  
  "dashboard": {
    "width": 800,                      // размер окна
    "height": 600,
    "camera_info_width": 350,          // размер info box
    "camera_info_height": 80
  },
  
  "warning_panel": {
    "width": 360,
    "color_animation_duration_ms": 150,
    "connected_opacity": 1.0,
    "disconnected_opacity": 0.7,
    "panel_opacity": 0.8,
    "overlay_opacity": 0.92
  }
}
```

### Загрузка конфига:

**C++:**
```cpp
config::SceneConfig sceneConfig = 
    config::SceneConfig::loadFromFile("config/scene_config.json");
auto* viewModel = 
    new viewmodels::SceneConfigViewModel(sceneConfig, parent);
context->setContextProperty("sceneConfig", viewModel);
```

**QML доступ:**
```qml
position: Qt.vector3d(
    xMeters * sceneConfig.scaleFactor,
    sceneConfig.markingYPosition,
    -yMeters * sceneConfig.scaleFactor
)
```


## 6. ПОТОК ДАННЫХ: ОТ ДЕТЕКТОРА К ВИЗУАЛИЗАЦИИ

### Шаг 1: Получение TCP сообщений

```cpp
TcpReaderWorker (в отдельном потоке)
    ↓
    parseProtobufMessage()  // proto_parser.h
    ↓
    ConnectionManager::handleMessage()
```

### Шаг 2: Обновление Domain моделей

```cpp
// В ConnectionManager::laneSummaryReceived()
domain::LaneState lane_state_;
lane_state_.updateFromProto(summary);  // обновление из протобуфа

// В ConnectionManager::markingObjectsReceived()
domain::MarkingObjectModel marking_model_;
marking_model_.updateFromProto(objects);  // список разметок

// Каждая разметка:
domain::MarkingObject obj;
obj.updateFromProto(proto_obj);  // копирование данных
```

### Шаг 3: Обновление ViewModels

```cpp
// ConnectionManager испускает сигналы:
emit markingModelUpdated();  // сигнал в QT
emit laneStateUpdated();

// Во вьюмодели (MarkingObjectListModel):
void updateFromDomain(const domain::MarkingObjectModel& model) {
    for (const auto& obj : model) {
        objects_.push_back(obj);
    }
    endResetModel();  // notify QML
    emit countChanged(size);
}
```

### Шаг 4: QML Repeater3D создает делегаты

```qml
Repeater3D {
    model: markingModel  // QAbstractListModel от C++

    delegate: MarkingLine {
        // Автоматически получает доступ к data() ролям
        xMeters: model.xMeters
        yMeters: model.yMeters
        // ...
    }
}
```

### Шаг 5: Визуализация в 3D

```qml
MarkingLine (Model #Cube)
    position: (x*scale, y, -z*scale)
    rotation: yawDeg
    opacity: confidence/100
    color: определено по lineColor
```

## 7. КЛЮЧЕВЫЕ КООРДИНАТНЫЕ СИСТЕМЫ

### Детектор → Протобуф
- X: слева-справа от машины
- Y: вперед-назад (положительное вперед)

### QML Сцена (después масштабирования)
- X: слева-справа (lane width) - **СОВПАДАЕТ**
- Y: вверх-вниз (вертикаль) - **ПЕРПЕНДИКУЛЯРНО**
- Z: вперед-назад - **ИНВЕРТИРОВАНО** (-yMeters)

### Причина инверсии Z:
```cpp
// Детектор говорит: y=5 это 5 метров впереди
// QML должна отрисовать это впереди (отрицательное Z)
position.z = -yMeters * scaleFactor;
```

## 8. МАТЕРИАЛЫ И ОСВЕЩЕНИЕ

### PrincipledMaterial параметры:

```qml
// Разметки (ярко светящиеся)
PrincipledMaterial {
    baseColor: "#ffd700"           // желтый
    emissiveFactor: Qt.vector3d(r, g, b) * emissiveScale
    roughness: 0.55-0.8            // зависит от стиля
    metalness: 0.0                 // не металлические
}

// Дорога (темная матовая)
PrincipledMaterial {
    baseColor: "#2d2d2d"           // темно-серая
    metalness: 0.0
    roughness: 0.9                 // очень матовая
    emissiveFactor: 0.07           // слабая подсветка
}

// Машина (яркая с отражением)
PrincipledMaterial {
    baseColor: "#00e0ff"           // bright cyan
    metalness: 0.05                // слегка металлическая
    roughness: 0.55
    emissiveFactor: (0.7, 0.9, 1.0) // интенсивное свечение
}
```

## 9. СИГНАЛЫ И СЛОТЫ АРХИТЕКТУРА

### Qt Signals (асинхронное обновление):

```cpp
ConnectionManager
    signal laneStateUpdated()  
    → AppController::onLaneStateUpdated()
        → SyncMonitor::updateDataTimestamp()

    signal markingModelUpdated()
    → AppController::onMarkingModelUpdated()
        → MarkingOverlayProcessor::updateMarkings()

    signal warningModelUpdated()

    signal stateChanged(State)
    → AppController::onDataConnectionStateChanged()
```

### QML Автоматическое обновление:

Когда QAbstractListModel вызывает `endResetModel()`:
1. Qt отправляет сигнал countChanged
2. QML Repeater3D перестраивает делегаты
3. Новые MarkingLine элементы добавляются в 3D сцену
4. Камера автоматически их отрисовывает


## 10. ВАЖНЫЕ ПАРАМЕТРЫ ДЛЯ НАСТРОЙКИ

### Видимость разметок:
- `marking.confidence_divisor`: 100.0
  - opacity = confidence / 100
  - confidence < 30: практически невидимо
  - confidence > 100: полная видимость

### Производительность:
- `road.center_dash_count`: 30 (количество штрихов)
  - Больше = больше геометрии
- `marking.y_position_above_plane`: 0.02
  - Z-fighting при значении < 0.01

### Масштаб:
- `scale.scale_factor`: 1.0
  - Глобальный множитель для всех координат
  - Влияет на производительность

### Камера:
- `camera.height`: 800 (высота в единицах сцены)
- `camera.distance`: 600 (расстояние впереди машины)
- Для изменения вида без R-клавиши, модифицировать эти значения
