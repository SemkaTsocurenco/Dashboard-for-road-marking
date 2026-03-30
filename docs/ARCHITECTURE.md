# Архитектура приложения

## Обзор архитектуры

Приложение построено с использованием **трёхслойной архитектуры MVVM** (Model-View-ViewModel) и следует принципам чистой архитектуры для разделения ответственности между компонентами.

## Архитектурные слои

### 1. Presentation Layer (UI)

**Назначение:** Отображение данных пользователю и обработка пользовательского ввода.

**Компоненты:**
- **Qt Widgets** — `MainWindow`, `DashboardWidget`
- **QML 3D** — `Dashboard.qml`, `CarScene.qml`, `MarkingLine.qml`
- **Video Widget** — `NetworkVideoWidget` с `MarkingOverlayProcessor`

**Технологии:**
- Qt Widgets для основного окна
- Qt Quick 3D для 3D визуализации
- GStreamer для видеопотоков

### 2. ViewModel Layer

**Назначение:** Преобразование доменных моделей в формат, удобный для отображения в UI.

**Компоненты:**

| Класс | Базовый класс | Назначение |
|-------|---------------|------------|
| `LaneStateViewModel` | `QObject` | Состояние полос для QML |
| `MarkingObjectListModel` | `QAbstractListModel` | Список объектов разметки |
| `WarningListModel` | `QAbstractListModel` | Список предупреждений |
| `FittedLineListModel` | `QAbstractListModel` | Полиномиальные линии |
| `SceneConfigViewModel` | `QObject` | Параметры 3D сцены |

**Особенности:**
- Все ViewModel используют `Q_PROPERTY` для реактивной связи с QML
- Наследуют `QAbstractListModel` для интеграции с `ListView`
- Автоматическое обновление UI через сигналы Qt

### 3. Domain Layer

**Назначение:** Бизнес-логика и доменные модели.

**Компоненты:**

| Класс | Назначение |
|-------|------------|
| `LaneState` | Состояние полос движения |
| `MarkingObject` | Объект дорожной разметки |
| `FittedLine` | Полиномиальная линия разметки |
| `Warning` | Предупреждение системы |
| `WarningEngine` | Генератор предупреждений |

**Принципы:**
- Чистые доменные модели без зависимостей от Qt
- Независимость от слоя представления
- Инкапсуляция бизнес-логики

### 4. Data Layer

**Назначение:** Получение и обработка внешних данных.

**Компоненты:**

| Компонент | Назначение |
|-----------|------------|
| `ConnectionManager` | Управление TCP соединением |
| `TcpReaderWorker` | Асинхронное чтение из сокета |
| `ProtoParser` | Парсинг бинарного протокола |
| `NetworkVideoWidget` | Получение видеопотока |

## Диаграмма компонентов

```
┌─────────────────────────────────────────────────────────────────────┐
│                         PRESENTATION LAYER                          │
│  ┌────────────────────────────────────────────────────────────────┐ │
│  │                         MainWindow                              │ │
│  │  ┌──────────────────┐  ┌────────────────┐  ┌────────────────┐  │ │
│  │  │  DashboardWidget │  │NetworkVideoWidget│ │ Status Panel   │  │ │
│  │  │  (QML 3D Scene)  │  │ (Video+Overlay)  │ │                │  │ │
│  │  └────────┬─────────┘  └───────┬─────────┘ └────────────────┘  │ │
│  └───────────┼────────────────────┼───────────────────────────────┘ │
└──────────────┼────────────────────┼─────────────────────────────────┘
               │                    │
┌──────────────▼────────────────────▼─────────────────────────────────┐
│                         VIEWMODEL LAYER                             │
│  ┌────────────────┐ ┌─────────────────────┐ ┌──────────────────┐   │
│  │LaneStateViewModel│MarkingObjectListModel│ │WarningListModel  │   │
│  └────────┬───────┘ └──────────┬──────────┘ └────────┬─────────┘   │
└───────────┼────────────────────┼─────────────────────┼──────────────┘
            │                    │                     │
┌───────────▼────────────────────▼─────────────────────▼──────────────┐
│                          DOMAIN LAYER                               │
│  ┌─────────────┐  ┌──────────────┐  ┌─────────┐  ┌──────────────┐  │
│  │  LaneState  │  │MarkingObject │  │ Warning │  │WarningEngine │  │
│  └─────────────┘  └──────────────┘  └─────────┘  └──────────────┘  │
└─────────────────────────────┬───────────────────────────────────────┘
                              │
┌─────────────────────────────▼───────────────────────────────────────┐
│                           DATA LAYER                                │
│  ┌──────────────────┐  ┌───────────────┐  ┌────────────────────┐   │
│  │ConnectionManager │  │ ProtoParser   │  │  Video Pipeline    │   │
│  │  (TCP Client)    │  │ (V2 Protocol) │  │  (GStreamer)       │   │
│  └──────────────────┘  └───────────────┘  └────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
```

## Паттерны проектирования

### 1. MVVM (Model-View-ViewModel)

```
View (QML)
    │
    │ Data Binding (Q_PROPERTY)
    ▼
ViewModel (LaneStateViewModel)
    │
    │ Transform
    ▼
Model (LaneState)
```

**Преимущества:**
- Разделение UI и бизнес-логики
- Тестируемость ViewModel без UI
- Реактивное обновление через Qt сигналы

### 2. Observer Pattern

```cpp
// ConnectionManager испускает сигналы
signals:
    void laneStateUpdated();
    void markingModelUpdated();

// AppController подписывается
connect(connectionManager, &ConnectionManager::laneStateUpdated,
        this, &AppController::onLaneStateUpdated);
```

**Использование:**
- Сигналы/слоты Qt для обновления данных
- Реактивное обновление UI при изменении моделей

### 3. State Machine

```cpp
// ProtoParser использует состояния
enum class ParserState {
    WaitingSync,      // Ожидание синхробайта 0xAA
    ReadingHeader,    // Чтение заголовка (9 байт)
    ReadingPayload,   // Чтение полезной нагрузки
    ReadingCrc        // Чтение контрольной суммы
};
```

**Использование:**
- `ProtoParser` — парсинг TCP протокола
- `ConnectionManager` — управление соединением

### 4. Adapter Pattern

```cpp
// proto_v2_adapter.h
namespace proto_v2_adapter {
    domain::LaneState toLaneState(const laneproto::LaneLines& lines);
    domain::MarkingObjectModel toMarkingModel(const laneproto::RoadObjects& objects);
}
```

**Использование:**
- Преобразование протокольных структур в доменные модели
- ViewModel адаптируют доменные модели для QML

### 5. Strategy Pattern

```cpp
// Интерфейс обработки видеокадров
class IVideoFrameProcessor {
public:
    virtual void processFrame(QImage& frame) = 0;
};

// Реализация для оверлея разметки
class MarkingOverlayProcessor : public IVideoFrameProcessor {
    void processFrame(QImage& frame) override;
};
```

## Потоки выполнения

### Main Thread

**Ответственность:**
- Qt Event Loop
- UI обновления
- QML рендеринг
- Обработка сигналов

### Worker Thread (TcpReaderWorker)

**Ответственность:**
- Чтение из TCP сокета
- Парсинг протокола
- Отправка сигналов в main thread

```
┌──────────────────┐        ┌──────────────────┐
│   Main Thread    │        │  Worker Thread   │
│                  │        │                  │
│  ConnectionMgr   │◄───────│ TcpReaderWorker  │
│  ViewModels      │ signal │  ProtoParser     │
│  QML Engine      │        │  Socket Read     │
│                  │        │                  │
└──────────────────┘        └──────────────────┘
```

### Синхронизация

```cpp
// Все обновления UI через сигналы/слоты с Qt::QueuedConnection
connect(worker, &TcpReaderWorker::laneLinesParsed,
        this, &ConnectionManager::onLaneLines,
        Qt::QueuedConnection);  // Thread-safe

// Mutex для защиты shared данных
QMutex m_mutex;
QMutexLocker locker(&m_mutex);
```

## Поток данных

### 1. Получение данных разметки

```
TCP Server
    │
    ▼ TCP Frame
TcpReaderWorker::readData()
    │
    ▼ Raw bytes
ProtoParser::feed()
    │
    ▼ Parsed message
IMessageHandler::onLaneLines() / onRoadObjects()
    │
    ▼ Signal
ConnectionManager::onLaneLines()
    │
    ▼ Domain Model
proto_v2_adapter::toLaneState()
    │
    ▼ Signal: laneStateUpdated()
AppController::onLaneStateUpdated()
    │
    ▼ Update
LaneStateViewModel::updateFromDomain()
    │
    ▼ Q_PROPERTY change
QML binding updates UI
```

### 2. Обработка видео с оверлеем

```
GStreamer Pipeline
    │
    ▼ Video Frame
QtMultimediaVideoProvider
    │
    ▼ QImage
MarkingOverlayProcessor::processFrame()
    │
    ├─ drawLaneOverlay()
    ├─ drawMarkingObjects()
    ├─ drawFittedLines()
    └─ drawWarnings()
    │
    ▼ Processed Frame
NetworkVideoWidget display
```

## Система координат

### Реальные координаты (метры)

```
        +X (вправо)
        ↑
        │
        │ Y (вперёд)
────────┼────────→
        │
        │
    Камера (0, 0)
```

### Координаты пикселей

```
(0,0) ─────→ +X
  │
  │
  ▼
  +Y
```

### 3D сцена (Qt Quick 3D)

```
Camera Position: (1000, 1000, -1000)
Camera Rotation: pitch=-20°, yaw=153°
View: Isometric with slight tilt
```

## Зависимости между модулями

```
main.cpp
    │
    ▼
AppController
    │
    ├── ConnectionManager ──► TcpReaderWorker ──► ProtoParser
    │
    ├── NetworkVideoWidget ──► MarkingOverlayProcessor
    │
    ├── LaneStateViewModel ◄── LaneState
    │
    ├── MarkingObjectListModel ◄── MarkingObject
    │
    ├── WarningListModel ◄── Warning ◄── WarningEngine
    │
    └── MainWindow
            │
            ├── DashboardWidget ──► QML (Dashboard.qml)
            │
            └── NetworkVideoWidget (embedded)
```

## Принципы расширения

### Добавление нового типа объекта разметки

1. Добавить enum в `domain/MarkingObject.h`
2. Обновить `proto_v2_adapter` для парсинга
3. Добавить визуализацию в `MarkingOverlayProcessor`
4. Добавить 3D компонент в QML

### Добавление нового типа предупреждения

1. Добавить enum в `domain/Warning.h`
2. Обновить `WarningEngine` для генерации
3. Обновить `WarningPanel.qml` для отображения

### Добавление нового источника данных

1. Реализовать интерфейс `IMessageHandler`
2. Создать новый Reader класс
3. Интегрировать в `ConnectionManager`
