# Модули приложения

## Обзор модулей

```
Dashboard-for-road-marking/
├── app/              # Контроллер приложения
├── config/           # Конфигурация
├── domain/           # Доменные модели
├── network/          # TCP соединение
├── parser/           # Парсер протокола
├── ui/               # UI компоненты
├── videowidget/      # Видео виджет
├── viewmodels/       # MVVM слой
└── logger/           # Логирование
```

---

## Модуль APP

**Путь:** `/app`

**Назначение:** Центральный контроллер и координация всех компонентов приложения.

### Файлы

| Файл | Описание |
|------|----------|
| `AppController.hpp` | Главный контроллер приложения |
| `AppController.cpp` | Реализация контроллера |
| `SynchronizationMonitor.hpp` | Мониторинг синхронизации данных и видео |
| `SynchronizationMonitor.cpp` | Реализация мониторинга |

### AppController

**Ответственность:**
- Инициализация всех компонентов
- Создание и связывание ViewModel'ей
- Управление жизненным циклом приложения
- Координация между слоями

**Основные методы:**

```cpp
class AppController : public QObject {
    Q_OBJECT

    // Q_PROPERTY для доступа из QML
    Q_PROPERTY(LaneStateViewModel* laneViewModel READ laneViewModel CONSTANT)
    Q_PROPERTY(MarkingObjectListModel* markingListModel READ markingListModel CONSTANT)
    Q_PROPERTY(WarningListModel* warningListModel READ warningListModel CONSTANT)
    Q_PROPERTY(bool isFullyConnected READ isFullyConnected NOTIFY connectionStateChanged)

public:
    // Инициализация с конфигурацией
    bool initialize(const QString& config_path,
                   const QString& scene_config_path);

    // Завершение работы
    void shutdown();

    // Доступ к компонентам
    ConnectionManager* connectionManager();
    NetworkVideoWidget* videoWidget();
    SynchronizationMonitor* syncMonitor();

    // ViewModel'ы
    LaneStateViewModel* laneViewModel();
    MarkingObjectListModel* markingListModel();
    WarningListModel* warningListModel();
    FittedLineListModel* fittedLineListModel();

signals:
    void connectionStateChanged();
    void initialized();
    void shutdownComplete();
};
```

### SynchronizationMonitor

**Ответственность:**
- Отслеживание разницы timestamp между данными и видео
- Генерация предупреждений о рассинхронизации

```cpp
class SynchronizationMonitor : public QObject {
    Q_OBJECT

public:
    void setMaxTimestampDiff(int ms);
    bool isSynchronized() const;
    int currentDiff() const;

signals:
    void syncStateChanged(bool synchronized);
    void syncWarning(const QString& message);
};
```

---

## Модуль CONFIG

**Путь:** `/config`

**Назначение:** Загрузка, валидация и управление конфигурацией.

### Файлы

| Файл | Описание |
|------|----------|
| `AppConfig.hpp/cpp` | Агрегатный класс конфигурации |
| `ConfigurationManager.hpp/cpp` | Менеджер загрузки конфигов |
| `SceneConfig.hpp/cpp` | Конфигурация 3D сцены |

### Структуры конфигурации

```cpp
// Конфигурация сети
struct NetworkConfig {
    QString host = "127.0.0.1";
    int port = 9000;
    bool auto_reconnect = true;
    int reconnect_interval_ms = 5000;
};

// Конфигурация видео
struct VideoConfig {
    QString source_url;
    bool auto_start = false;
};

// Конфигурация предупреждений
struct WarningConfig {
    float lane_departure_threshold_m = 0.3f;
    float crosswalk_distance_threshold_m = 30.0f;
    float crosswalk_critical_distance_m = 10.0f;
    bool enable_crosswalk_warnings = true;
    bool enable_lane_departure_warnings = true;
};

// Агрегатный класс
class AppConfig {
public:
    static AppConfig loadFromFile(const QString& path);

    NetworkConfig network;
    VideoConfig video;
    VideoProcessingConfig video_processing;
    WarningConfig warning;
    SyncConfig sync;
    UIConfig ui;
};
```

---

## Модуль DOMAIN

**Путь:** `/domain`

**Назначение:** Доменные модели и бизнес-логика.

### Файлы

| Файл | Описание |
|------|----------|
| `LaneState.h/cpp` | Состояние полос движения |
| `MarkingObject.h/cpp` | Объект дорожной разметки |
| `FittedLine.h/cpp` | Полиномиальная линия |
| `Warning.h/cpp` | Предупреждение системы |
| `WarningEngine.h/cpp` | Генератор предупреждений |

### LaneState

Представляет состояние полос движения.

```cpp
class LaneState {
public:
    // Типы линий
    enum class LineType {
        Unknown = 0,
        Solid = 1,
        Dashed = 2,
        Double = 3
    };

    // Цвета линий
    enum class LineColor {
        Unknown = 0,
        White = 1,
        Yellow = 2,
        Red = 3
    };

    // Данные о линии
    struct LineInfo {
        LineType type = LineType::Unknown;
        LineColor color = LineColor::Unknown;
        float offset_m = 0.0f;      // Смещение от центра
        float confidence = 0.0f;    // Уверенность детекции
    };

    LineInfo left_line;
    LineInfo right_line;
    float center_offset_m = 0.0f;   // Смещение авто от центра
    int quality = 0;                // Качество детекции (0-100)
    uint64_t timestamp_ms = 0;      // Временная метка

    bool isValid() const;
};
```

### MarkingObject

Представляет объект дорожной разметки.

```cpp
class MarkingObject {
public:
    // Типы объектов
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

    // Координаты в метрах (реальный мир)
    float x_meters = 0.0f;
    float y_meters = 0.0f;
    float width_meters = 0.0f;
    float height_meters = 0.0f;

    // Координаты в пикселях (для оверлея)
    int center_x_pixels = 0;
    int center_y_pixels = 0;
    int width_pixels = 0;
    int height_pixels = 0;

    float confidence = 0.0f;
    uint64_t timestamp_ms = 0;

    // Вспомогательные методы
    bool isCrosswalk() const;
    bool isArrow() const;
    bool isStopLine() const;
    QString className() const;
};

// Модель списка объектов
class MarkingObjectModel {
public:
    void addObject(const MarkingObject& obj);
    void clear();
    const std::vector<MarkingObject>& objects() const;

    // Поиск ближайшего объекта типа
    std::optional<MarkingObject> nearestCrosswalk() const;
    std::optional<MarkingObject> nearestArrow() const;
};
```

### FittedLine

Полиномиальная линия разметки: `x = a*y² + b*y + c`

```cpp
class FittedLine {
public:
    // Полиномиальные коэффициенты
    float a = 0.0f;  // y²
    float b = 0.0f;  // y
    float c = 0.0f;  // constant

    float confidence = 0.0f;
    int line_id = 0;

    // Вычисление точки на линии
    float calculateX(float y) const {
        return a * y * y + b * y + c;
    }

    // Генерация точек для отрисовки
    std::vector<QPointF> generatePoints(float y_start, float y_end, int count) const;
};

class FittedLinesModel {
public:
    std::vector<FittedLine> lines;
    uint64_t timestamp_ms = 0;
};
```

### Warning

Представляет предупреждение системы.

```cpp
class Warning {
public:
    enum class Type {
        None,
        CrosswalkAhead,
        LaneDepartureLeft,
        LaneDepartureRight,
        SolidLineCrossing
    };

    enum class Severity {
        Info,
        Warning,
        Critical
    };

    Type type = Type::None;
    Severity severity = Severity::Info;
    QString message;
    float distance_m = 0.0f;
    uint64_t timestamp_ms = 0;

    QString icon() const;
    QColor color() const;
};
```

### WarningEngine

Генератор предупреждений на основе состояния полос и объектов.

```cpp
class WarningEngine {
public:
    struct Config {
        float lane_departure_offset_threshold_m = 0.3f;
        float crosswalk_distance_threshold_m = 30.0f;
        float crosswalk_critical_distance_m = 10.0f;
        bool enable_crosswalk_warnings = true;
        bool enable_lane_departure_warnings = true;
    };

    void setConfig(const Config& config);

    // Генерация предупреждений
    std::vector<Warning> update(
        const LaneState& lane,
        const MarkingObjectModel& markings,
        uint64_t timestamp_ms
    ) const;

private:
    Warning checkLaneDeparture(const LaneState& lane) const;
    Warning checkCrosswalk(const MarkingObjectModel& markings) const;
};
```

---

## Модуль NETWORK

**Путь:** `/network`

**Назначение:** Управление TCP соединением для получения данных разметки.

### Файлы

| Файл | Описание |
|------|----------|
| `ConnectionManager.h/cpp` | Менеджер TCP соединения |
| `TcpReaderWorker.h/cpp` | Worker для чтения из сокета |

### ConnectionManager

**Ответственность:**
- Управление TCP соединением
- Обработка переподключения
- Распространение данных через сигналы

```cpp
class ConnectionManager : public QObject {
    Q_OBJECT

public:
    enum class State {
        Disconnected,
        Connecting,
        Connected,
        Disconnecting,
        Reconnecting,
        Error
    };
    Q_ENUM(State)

    // Управление соединением
    Q_INVOKABLE void connectToHost(const QString& host, int port);
    Q_INVOKABLE void disconnectFromHost();

    // Настройки
    void setAutoReconnect(bool enabled);
    void setReconnectInterval(int ms);

    // Состояние
    State state() const;
    bool isConnected() const;
    QString lastError() const;

    // Доступ к данным
    const domain::LaneState& laneState() const;
    const domain::MarkingObjectModel& markingModel() const;
    const domain::FittedLinesModel& fittedLines() const;

signals:
    // Состояние соединения
    void connectedChanged(bool connected);
    void stateChanged(State state);
    void errorOccurred(const QString& error);

    // Обновление данных
    void laneStateUpdated();
    void markingModelUpdated();
    void fittedLinesUpdated();
    void warningModelUpdated();

    // Сырые данные протокола
    void laneLinesReceived(const laneproto::LaneLines& lines);
    void roadObjectsReceived(const laneproto::RoadObjects& objects);

private slots:
    void onLaneLines(const laneproto::LaneLines& lines);
    void onRoadObjects(const laneproto::RoadObjects& objects);
    void onWorkerError(const QString& error);
    void onReconnectTimer();
};
```

### TcpReaderWorker

**Ответственность:**
- Асинхронное чтение из TCP сокета в отдельном потоке
- Передача данных в ProtoParser
- Отправка сигналов при получении сообщений

```cpp
class TcpReaderWorker : public QObject {
    Q_OBJECT

public:
    void setSocket(QTcpSocket* socket);
    void setParser(ProtoParser* parser);

public slots:
    void start();
    void stop();
    void readData();

signals:
    void laneLinesParsed(const laneproto::LaneLines& lines);
    void roadObjectsParsed(const laneproto::RoadObjects& objects);
    void errorOccurred(const QString& error);
    void finished();

private:
    QTcpSocket* m_socket = nullptr;
    ProtoParser* m_parser = nullptr;
    bool m_running = false;
};
```

---

## Модуль PARSER

**Путь:** `/parser`

**Назначение:** Парсинг бинарного TCP протокола V2.

### Файлы

| Файл | Описание |
|------|----------|
| `proto_parser.h/cpp` | Конечный автомат парсера |
| `proto_structures_v2.h` | Структуры данных протокола |
| `proto_v2_adapter.h/cpp` | Адаптер в доменные модели |

### ProtoParser

Конечный автомат для парсинга потока байтов.

```cpp
class ProtoParser {
public:
    // Интерфейс обработчика сообщений
    class IMessageHandler {
    public:
        virtual void onLaneLines(const laneproto::LaneLines& msg) = 0;
        virtual void onRoadObjects(const laneproto::RoadObjects& msg) = 0;
    };

    void setHandler(IMessageHandler* handler);

    // Подача данных на вход парсера
    void feed(const uint8_t* data, size_t size);
    void feed(const QByteArray& data);

    // Сброс состояния
    void reset();

    // Статистика
    uint64_t framesReceived() const;
    uint64_t framesDropped() const;

private:
    enum class State {
        WaitingSync,      // Ожидание 0xAA
        ReadingHeader,    // Чтение 9 байт заголовка
        ReadingPayload,   // Чтение payload
        ReadingCrc        // Чтение 2 байт CRC
    };

    State m_state = State::WaitingSync;
    std::vector<uint8_t> m_buffer;
    laneproto::Header m_header;
    IMessageHandler* m_handler = nullptr;
};
```

### Структуры протокола

```cpp
namespace laneproto {

// Заголовок кадра
struct Header {
    uint8_t version;        // Версия протокола
    uint8_t msg_type;       // Тип сообщения
    uint8_t sequence;       // Номер последовательности
    uint32_t timestamp_ms;  // Временная метка
    uint16_t payload_len;   // Длина payload
};

// Линия разметки
struct LaneLine {
    uint8_t line_id;
    uint8_t style;          // Solid=1, Dashed=2, Double=3
    uint8_t color;          // White=1, Yellow=2, Red=3
    float offset_m;         // Смещение от центра
    float confidence;       // Уверенность (0-100)
    float poly_a, poly_b, poly_c;  // Полиномиальные коэффициенты
};

// Сообщение о линиях
struct LaneLines {
    uint8_t line_count;
    std::vector<LaneLine> lines;
    float center_offset_m;
    uint8_t quality;
};

// Объект разметки
struct RoadObject {
    uint8_t class_id;
    float x_m, y_m;
    float width_m, height_m;
    int16_t center_x_px, center_y_px;
    int16_t width_px, height_px;
    float confidence;
};

// Сообщение об объектах
struct RoadObjects {
    uint8_t object_count;
    std::vector<RoadObject> objects;
};

} // namespace laneproto
```

### proto_v2_adapter

Адаптер для преобразования протокольных структур в доменные модели.

```cpp
namespace proto_v2_adapter {

// Преобразование в LaneState
domain::LaneState toLaneState(const laneproto::LaneLines& lines);

// Преобразование в MarkingObjectModel
domain::MarkingObjectModel toMarkingModel(const laneproto::RoadObjects& objects);

// Преобразование в FittedLinesModel
domain::FittedLinesModel toFittedLines(const laneproto::LaneLines& lines);

// Вспомогательные преобразования
domain::LaneState::LineType toLineType(uint8_t style);
domain::LaneState::LineColor toLineColor(uint8_t color);
domain::MarkingObject::ClassId toClassId(uint8_t class_id);

} // namespace proto_v2_adapter
```

---

## Модуль VIEWMODELS

**Путь:** `/viewmodels`

**Назначение:** Преобразование доменных моделей для отображения в UI.

### Файлы

| Файл | Описание |
|------|----------|
| `LaneStateViewModel.h/cpp` | ViewModel для состояния полос |
| `MarkingObjectListModel.h/cpp` | Модель списка объектов |
| `WarningListModel.h/cpp` | Модель списка предупреждений |
| `FittedLineListModel.h/cpp` | Модель списка линий |
| `SceneConfigViewModel.hpp/cpp` | ViewModel конфигурации сцены |

### LaneStateViewModel

```cpp
class LaneStateViewModel : public QObject {
    Q_OBJECT

    // Основные свойства
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)
    Q_PROPERTY(float centerOffset READ centerOffset NOTIFY centerOffsetChanged)
    Q_PROPERTY(int quality READ quality NOTIFY qualityChanged)

    // Левая линия
    Q_PROPERTY(int laneTypeLeft READ laneTypeLeft NOTIFY laneTypeLeftChanged)
    Q_PROPERTY(int laneColorLeft READ laneColorLeft NOTIFY laneColorLeftChanged)
    Q_PROPERTY(float leftOffset READ leftOffset NOTIFY leftOffsetChanged)
    Q_PROPERTY(float leftConfidence READ leftConfidence NOTIFY leftConfidenceChanged)

    // Правая линия
    Q_PROPERTY(int laneTypeRight READ laneTypeRight NOTIFY laneTypeRightChanged)
    Q_PROPERTY(int laneColorRight READ laneColorRight NOTIFY laneColorRightChanged)
    Q_PROPERTY(float rightOffset READ rightOffset NOTIFY rightOffsetChanged)
    Q_PROPERTY(float rightConfidence READ rightConfidence NOTIFY rightConfidenceChanged)

public:
    // Обновление из доменной модели
    void updateFromDomain(const domain::LaneState& state);

    // Геттеры
    bool valid() const;
    float centerOffset() const;
    int quality() const;
    // ... и т.д.

signals:
    void validChanged();
    void centerOffsetChanged();
    void qualityChanged();
    void laneTypeLeftChanged();
    // ... и т.д.
};
```

### MarkingObjectListModel

```cpp
class MarkingObjectListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        ClassIdRole = Qt::UserRole + 1,
        ClassNameRole,
        XMetersRole,
        YMetersRole,
        WidthMetersRole,
        HeightMetersRole,
        ConfidenceRole,
        IsCrosswalkRole,
        IsArrowRole,
        CenterXPixelsRole,
        CenterYPixelsRole,
        WidthPixelsRole,
        HeightPixelsRole
    };

    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Обновление данных
    void updateFromDomain(const domain::MarkingObjectModel& model);
    int count() const;

signals:
    void countChanged();

private:
    std::vector<domain::MarkingObject> m_objects;
};
```

### WarningListModel

```cpp
class WarningListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool hasWarnings READ hasWarnings NOTIFY hasWarningsChanged)

public:
    enum Roles {
        TypeRole = Qt::UserRole + 1,
        SeverityRole,
        MessageRole,
        DistanceRole,
        IconRole,
        ColorRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void updateWarnings(const std::vector<domain::Warning>& warnings);

    int count() const;
    bool hasWarnings() const;

signals:
    void countChanged();
    void hasWarningsChanged();

private:
    std::vector<domain::Warning> m_warnings;
};
```

---

## Модуль UI

**Путь:** `/ui`

**Назначение:** Пользовательский интерфейс (Qt Widgets + QML).

### Файлы

| Файл | Описание |
|------|----------|
| `MainWindow.hpp/cpp` | Главное окно приложения |
| `DashboardWidget.hpp/cpp` | Контейнер для QML сцены |
| `qml/` | QML компоненты |

### MainWindow

```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

    void setAppController(AppController* controller);
    void setVideoWidget(NetworkVideoWidget* widget);

protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onConnectionStateChanged();
    void onVideoStateChanged();
    void updateStatusBar();

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupConnections();

    AppController* m_controller = nullptr;
    DashboardWidget* m_dashboardWidget = nullptr;
    NetworkVideoWidget* m_videoWidget = nullptr;
    QLabel* m_connectionStatus = nullptr;
    QLabel* m_videoStatus = nullptr;
};
```

### DashboardWidget

Контейнер для загрузки и отображения QML 3D сцены.

```cpp
class DashboardWidget : public QQuickWidget {
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget* parent = nullptr);

    void setAppController(AppController* controller);
    void setSceneConfig(SceneConfigViewModel* config);

private:
    void setupQmlEngine();
    void loadQmlScene();

    AppController* m_controller = nullptr;
    SceneConfigViewModel* m_sceneConfig = nullptr;
};
```

---

## Модуль VIDEOWIDGET

**Путь:** `/videowidget`

**Назначение:** Видеопоток и наложение разметки.

### Структура

```
videowidget/
├── base/
│   └── AbstractVideoWidget.hpp/cpp
├── interfaces/
│   ├── IVideoFrameProvider.hpp
│   └── IVideoFrameProcessor.hpp
├── src/
│   ├── QtMultimediaVideoProvider.hpp/cpp
│   └── BasicFrameHandle.hpp/cpp
├── widgets/
│   └── NetworkVideoWidget.hpp/cpp
└── processors/
    └── MarkingOverlayProcessor.hpp/cpp
```

### NetworkVideoWidget

```cpp
class NetworkVideoWidget : public AbstractVideoWidget {
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

public:
    explicit NetworkVideoWidget(QWidget* parent = nullptr);

    // Управление подключением
    Q_INVOKABLE void connectToSource(const QString& url);
    Q_INVOKABLE void disconnect();

    // Настройка процессора
    void setFrameProcessor(IVideoFrameProcessor* processor);

    // Состояние
    bool isConnected() const;
    QString currentUrl() const;

signals:
    void connectedChanged(bool connected);
    void connectionFailed(const QString& error);
    void frameDisplayed(quint64 timestamp_ms);

private:
    void setupGStreamerPipeline(const QString& url);
    void processFrame(const QImage& frame);

    IVideoFrameProvider* m_provider = nullptr;
    IVideoFrameProcessor* m_processor = nullptr;
    QString m_currentUrl;
};
```

### MarkingOverlayProcessor

```cpp
class MarkingOverlayProcessor : public IVideoFrameProcessor {
public:
    // Установка источников данных
    void setLaneStateViewModel(LaneStateViewModel* vm);
    void setMarkingObjectListModel(MarkingObjectListModel* model);
    void setFittedLineListModel(FittedLineListModel* model);
    void setWarningListModel(WarningListModel* model);

    // Обновление данных
    void updateMarkings(const domain::MarkingObjectModel& model);
    void updateFittedLines(const domain::FittedLinesModel& model);

    // Настройки оверлея
    void setSourceSize(int width, int height);
    void setFittedLinePointsCount(int count);

    // IVideoFrameProcessor interface
    void processFrame(QImage& frame) override;

private:
    void drawOverlay(QImage& image);
    void drawLaneOverlay(QPainter& painter, const QSize& size);
    void drawMarkingObjects(QPainter& painter, const QSize& size);
    void drawFittedLines(QPainter& painter, const QSize& size);
    void drawWarnings(QPainter& painter, const QSize& size);

    // Преобразование координат
    QPointF metersToPixels(float x_m, float y_m, const QSize& imageSize);
    QPointF sourceToDisplay(int x, int y, const QSize& imageSize);

    mutable QMutex m_mutex;
    LaneStateViewModel* m_laneVM = nullptr;
    MarkingObjectListModel* m_markingModel = nullptr;
    // ...
};
```

---

## Модуль LOGGER

**Путь:** `/logger`

**Назначение:** Логирование событий приложения.

### Файлы

| Файл | Описание |
|------|----------|
| `Logger.hpp/cpp` | Основной класс логгера |
| `LoggerMacros.hpp` | Макросы для логирования |

### Использование

```cpp
#include "logger/LoggerMacros.hpp"

// Уровни логирования
LOG_DEBUG("Parser state: {}", state);
LOG_INFO("Connected to {}:{}", host, port);
LOG_WARN("Reconnecting in {} ms", interval);
LOG_ERROR("Connection failed: {}", error);

// Условное логирование
LOG_DEBUG_IF(verbose, "Detailed info: {}", details);
```

### Logger

```cpp
class Logger {
public:
    enum class Level {
        Debug,
        Info,
        Warning,
        Error
    };

    static Logger& instance();

    void setLevel(Level level);
    void setOutputFile(const QString& path);
    void setConsoleOutput(bool enabled);

    void log(Level level, const QString& message);
    void log(Level level, const char* format, ...);

private:
    Logger() = default;
    Level m_level = Level::Info;
    QFile m_file;
    bool m_consoleOutput = true;
    QMutex m_mutex;
};
```
