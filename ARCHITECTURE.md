# Dashboard Architecture - Подробная спецификация реализации

## Оглавление
1. [Общий обзор](#общий-обзор)
2. [Структура директорий](#структура-директорий)
3. [Компоненты системы](#компоненты-системы)
4. [Детальная спецификация компонентов](#детальная-спецификация-компонентов)
5. [Потоки данных](#потоки-данных)
6. [Конфигурация](#конфигурация)
7. [План реализации](#план-реализации)
8. [Контракты и интерфейсы](#контракты-и-интерфейсы)

---

## Общий обзор

### Архитектурные решения

**Выбранный подход**: Application Controller с отдельным Configuration Manager

**Ключевые принципы**:
- **Eager initialization**: Все компоненты создаются при старте приложения
- **Single Responsibility**: Каждый компонент имеет четко определенную ответственность
- **Separation of Concerns**: UI (MainWindow) отделен от бизнес-логики (AppController)
- **Configuration-driven**: Все параметры загружаются из JSON файла при старте
- **Гибридный UI**: Qt Widgets для основного окна + QML для отдельных компонентов

### Основные требования

1. **UI**: Гибридный подход (Widgets + QML)
2. **Источники данных**: Один активный, с возможностью переключения
3. **Конфигурация**: JSON файлы, загружаются при старте
4. **Lifecycle**: Eager - все компоненты создаются сразу при initialize()
5. **Синхронизация**: Жесткая - данные и видео должны соответствовать по timestamp
6. **Частичный сбой**: Работать с тем что есть (данные без видео или наоборот)
7. **Runtime изменения**: Не поддерживаются, требуется перезапуск

---

## Структура директорий

```
Dashboard-for-road-marking/
├── app/                          # [НОВАЯ] Application layer
│   ├── AppController.hpp
│   ├── AppController.cpp
│   ├── SynchronizationMonitor.hpp
│   └── SynchronizationMonitor.cpp
│
├── config/                       # [НОВАЯ] Configuration management
│   ├── ConfigurationManager.hpp
│   ├── ConfigurationManager.cpp
│   ├── AppConfig.hpp             # Структуры конфигурации
│   └── config.json               # Файл конфигурации по умолчанию
│
├── ui/                           # [НОВАЯ] UI components
│   ├── MainWindow.hpp
│   ├── MainWindow.cpp
│   ├── MainWindow.ui             # Qt Designer UI file
│   └── qml/                      # QML компоненты
│       ├── VideoPanel.qml
│       ├── LaneStatePanel.qml
│       └── WarningListPanel.qml
│
├── domain/                       # [СУЩЕСТВУЮЩАЯ] Domain models
│   ├── LaneState.h/cpp
│   ├── MarkingObject.h/cpp
│   ├── Warning.h/cpp
│   └── WarningEngine.h/cpp
│
├── network/                      # [СУЩЕСТВУЮЩАЯ] Network layer
│   ├── ConnectionManager.h/cpp
│   └── TcpReaderWorker.h/cpp
│
├── videowidget/                  # [СУЩЕСТВУЮЩАЯ] Video components
│   ├── widgets/
│   │   └── NetworkVideoWidget.hpp/cpp
│   ├── processors/
│   │   └── MarkingOverlayProcessor.hpp/cpp
│   ├── interfaces/
│   └── src/
│
├── viewmodels/                   # [СУЩЕСТВУЮЩАЯ] ViewModels для UI
│   ├── LaneStateViewModel.h/cpp
│   ├── MarkingObjectListModel.h/cpp
│   └── WarningListModel.h/cpp
│
├── logger/                       # [СУЩЕСТВУЮЩАЯ] Logging
│   ├── Logger.hpp/cpp
│   └── LoggerMacros.hpp
│
├── parser/                       # [СУЩЕСТВУЮЩАЯ] Protocol parsing
│   └── proto_parser.h/cpp
│
├── main.cpp                      # [МОДИФИЦИРОВАТЬ] Entry point
├── CMakeLists.txt                # [МОДИФИЦИРОВАТЬ] Build configuration
└── ARCHITECTURE.md               # [ЭТОТ ФАЙЛ]
```

---

## Компоненты системы

### Диаграмма компонентов

```
┌─────────────────────────────────────────────────────────────────┐
│                           main.cpp                               │
│  - Инициализация QApplication                                    │
│  - Создание AppController                                        │
│  - Создание MainWindow                                           │
│  - Запуск event loop                                             │
└────────────────────────┬────────────────────────────────────────┘
                         │
         ┌───────────────┴───────────────┐
         ↓                               ↓
┌─────────────────────────┐   ┌──────────────────────────────┐
│    AppController        │   │      MainWindow              │
│  ┌──────────────────┐   │   │  - Widgets UI                │
│  │ ВЛАДЕЕТ:         │   │   │  - QML контейнеры            │
│  │ - ConnMgr        │◄──┼───│  - Подключается к ViewModels │
│  │ - VideoWidget    │   │   │  - Отображает данные         │
│  │ - MarkOverlay    │   │   │  - События пользователя      │
│  │ - SyncMonitor    │   │   └──────────────────────────────┘
│  │ - ConfigMgr      │   │
│  └──────────────────┘   │
│                         │
│  КООРДИНИРУЕТ:          │
│  - Инициализацию        │
│  - Связывание компонентов│
│  - Потоки данных        │
│  - Глобальное состояние │
└─────────────────────────┘
         │
    ┌────┼─────┬───────────────┬─────────────┬──────────────┐
    ↓    ↓     ↓               ↓             ↓              ↓
┌────────┐┌────────┐┌────────────┐┌───────────┐┌──────────────┐
│ConnMgr ││Video   ││MarkOverlay ││ConfigMgr  ││SyncMonitor   │
│        ││Widget  ││Processor   ││           ││              │
│Network ││RTSP    ││Рисует      ││JSON       ││Проверяет     │
│данные  ││видео   ││разметку    ││конфиг     ││timestamp     │
│        ││        ││на видео    ││           ││разницу       │
└────────┘└────────┘└────────────┘└───────────┘└──────────────┘
    │        │           │
    ↓        ↓           ↓
┌──────────────────────────┐
│     ViewModels           │
│ - LaneStateViewModel     │
│ - MarkingObjectListModel │
│ - WarningListModel       │
└──────────────────────────┘
```

---

## Детальная спецификация компонентов

### 1. ConfigurationManager

**Файл**: `config/ConfigurationManager.hpp`, `config/ConfigurationManager.cpp`

**Назначение**: Загрузка, валидация и предоставление конфигурации приложения

**Зависимости**:
- Qt Core (QJsonDocument, QFile)
- Logger

**Структуры данных**:

```cpp
namespace config {

// Конфигурация сетевого подключения
struct NetworkConfig {
    QString host{"127.0.0.1"};
    quint16 port{5000};
    int reconnect_interval_ms{5000};
    int max_reconnect_attempts{0};  // 0 = unlimited
    bool auto_reconnect{true};

    QJsonObject toJson() const;
    static NetworkConfig fromJson(const QJsonObject& json);
};

// Конфигурация видео
struct VideoConfig {
    QString source_url{"rtsp://127.0.0.1:8554/stream"};
    bool auto_start{false};

    QJsonObject toJson() const;
    static VideoConfig fromJson(const QJsonObject& json);
};

// Конфигурация WarningEngine
struct WarningConfig {
    float lane_departure_threshold_m{0.3f};
    float crosswalk_distance_threshold_m{30.0f};
    float crosswalk_critical_distance_m{10.0f};
    std::uint8_t min_marking_confidence{50};
    std::uint8_t min_lane_quality{60};
    bool enable_crosswalk_warnings{true};
    bool enable_lane_departure_warnings{true};

    QJsonObject toJson() const;
    static WarningConfig fromJson(const QJsonObject& json);

    // Конвертация в domain::WarningEngineConfig
    domain::WarningEngineConfig toDomainConfig() const;
};

// Конфигурация синхронизации
struct SyncConfig {
    int max_timestamp_diff_ms{500};  // Порог для warning
    bool enable_sync_monitoring{true};

    QJsonObject toJson() const;
    static SyncConfig fromJson(const QJsonObject& json);
};

// Общая конфигурация приложения
struct AppConfig {
    NetworkConfig network;
    VideoConfig video;
    WarningConfig warning;
    SyncConfig sync;

    QJsonObject toJson() const;
    static AppConfig fromJson(const QJsonObject& json);
};

// Менеджер конфигурации
class ConfigurationManager {
public:
    // Загрузка конфигурации из файла
    static AppConfig loadFromFile(const QString& path);

    // Сохранение конфигурации в файл
    static bool saveToFile(const QString& path, const AppConfig& config);

    // Конфигурация по умолчанию
    static AppConfig defaultConfig();

private:
    // Валидация конфигурации
    static bool validateConfig(const AppConfig& config, QString& error);

    // Валидация отдельных секций
    static bool validateNetworkConfig(const NetworkConfig& cfg, QString& error);
    static bool validateVideoConfig(const VideoConfig& cfg, QString& error);
    static bool validateWarningConfig(const WarningConfig& cfg, QString& error);
    static bool validateSyncConfig(const SyncConfig& cfg, QString& error);
};

} // namespace config
```

**Реализация**:

Ключевые моменты реализации:

1. **loadFromFile()**:
   - Открыть файл
   - Парсить JSON
   - Валидировать структуру
   - Создать AppConfig из JSON
   - Валидировать значения
   - Если ошибка - выбросить исключение с детальным описанием

2. **Валидация**:
   - Проверка диапазонов значений (port: 1-65535, thresholds > 0)
   - Проверка обязательных полей
   - Проверка логической корректности (critical_distance < distance_threshold)

3. **Значения по умолчанию**:
   - Использовать разумные defaults
   - Документировать в комментариях почему выбраны эти значения

**Пример JSON конфигурации**:

```json
{
  "network": {
    "host": "192.168.1.100",
    "port": 5000,
    "reconnect_interval_ms": 5000,
    "max_reconnect_attempts": 0,
    "auto_reconnect": true
  },
  "video": {
    "source_url": "rtsp://192.168.1.100:8554/stream",
    "auto_start": false
  },
  "warning": {
    "lane_departure_threshold_m": 0.3,
    "crosswalk_distance_threshold_m": 30.0,
    "crosswalk_critical_distance_m": 10.0,
    "min_marking_confidence": 50,
    "min_lane_quality": 60,
    "enable_crosswalk_warnings": true,
    "enable_lane_departure_warnings": true
  },
  "sync": {
    "max_timestamp_diff_ms": 500,
    "enable_sync_monitoring": true
  }
}
```

---

### 2. SynchronizationMonitor

**Файл**: `app/SynchronizationMonitor.hpp`, `app/SynchronizationMonitor.cpp`

**Назначение**: Мониторинг синхронизации timestamp между данными и видео

**Зависимости**:
- Qt Core (QObject, signals/slots)
- Logger

**API**:

```cpp
namespace app {

class SynchronizationMonitor : public QObject {
    Q_OBJECT

    // Текущая разница timestamp (в миллисекундах)
    Q_PROPERTY(int timestampDiffMs READ timestampDiffMs
               NOTIFY timestampDiffChanged)

    // Статус синхронизации (true = в пределах порога)
    Q_PROPERTY(bool isSynchronized READ isSynchronized
               NOTIFY synchronizationChanged)

    // Timestamp последних данных
    Q_PROPERTY(quint64 lastDataTimestamp READ lastDataTimestamp
               NOTIFY dataTimestampChanged)

    // Timestamp последнего видео кадра
    Q_PROPERTY(quint64 lastVideoTimestamp READ lastVideoTimestamp
               NOTIFY videoTimestampChanged)

public:
    explicit SynchronizationMonitor(int threshold_ms, QObject* parent = nullptr);
    ~SynchronizationMonitor() override = default;

    // Обновление timestamp от данных
    void updateDataTimestamp(std::uint64_t timestamp_ms);

    // Обновление timestamp от видео
    void updateVideoTimestamp(std::uint64_t timestamp_ms);

    // Сброс состояния
    void reset();

    // Getters
    int timestampDiffMs() const { return timestamp_diff_ms_; }
    bool isSynchronized() const { return is_synchronized_; }
    quint64 lastDataTimestamp() const { return data_timestamp_ms_; }
    quint64 lastVideoTimestamp() const { return video_timestamp_ms_; }
    int threshold() const { return threshold_ms_; }

signals:
    void timestampDiffChanged(int diff_ms);
    void synchronizationChanged(bool synced);
    void dataTimestampChanged(quint64 timestamp);
    void videoTimestampChanged(quint64 timestamp);

    // Warning когда рассинхронизация превышает порог
    void desyncWarning(const QString& message);

    // Восстановление синхронизации
    void syncRestored();

private:
    std::uint64_t data_timestamp_ms_{0};
    std::uint64_t video_timestamp_ms_{0};
    int timestamp_diff_ms_{0};
    int threshold_ms_;
    bool is_synchronized_{true};
    bool has_data_{false};
    bool has_video_{false};

    void checkSynchronization();
    void setTimestampDiff(int diff);
    void setSynchronized(bool synced);
};

} // namespace app
```

**Логика работы**:

1. **updateDataTimestamp()**:
   - Сохранить timestamp
   - Установить has_data_ = true
   - Вызвать checkSynchronization()

2. **updateVideoTimestamp()**:
   - Сохранить timestamp
   - Установить has_video_ = true
   - Вызвать checkSynchronization()

3. **checkSynchronization()**:
   - Если нет данных или видео - выйти
   - Вычислить разницу: abs(data_timestamp - video_timestamp)
   - Обновить timestamp_diff_ms_ (emit signal если изменилось)
   - Проверить порог:
     - Если diff > threshold && was_synchronized:
       - Установить is_synchronized_ = false
       - emit desyncWarning()
     - Если diff <= threshold && was_not_synchronized:
       - Установить is_synchronized_ = true
       - emit syncRestored()

**Пример использования**:

```cpp
// В AppController
connect(connection_manager_, &ConnectionManager::laneStateUpdated,
        this, [this]() {
            const auto& state = connection_manager_->laneState();
            sync_monitor_->updateDataTimestamp(state.timestampMs());
        });

connect(video_widget_, &NetworkVideoWidget::frameDisplayed,
        this, [this](quint64 frame_timestamp) {
            sync_monitor_->updateVideoTimestamp(frame_timestamp);
        });

connect(sync_monitor_, &SynchronizationMonitor::desyncWarning,
        this, [this](const QString& msg) {
            LOG_WARN << "Desync: " << msg.toStdString();
            // Показать warning в UI
        });
```

---

### 3. AppController

**Файл**: `app/AppController.hpp`, `app/AppController.cpp`

**Назначение**: Центральный координатор приложения

**Зависимости**:
- ConfigurationManager
- ConnectionManager
- NetworkVideoWidget
- MarkingOverlayProcessor
- SynchronizationMonitor
- Logger

**API**:

```cpp
namespace app {

class AppController : public QObject {
    Q_OBJECT

    // ViewModels для прямого доступа из QML/UI
    Q_PROPERTY(viewmodels::LaneStateViewModel* laneViewModel
               READ laneViewModel CONSTANT)
    Q_PROPERTY(viewmodels::MarkingObjectListModel* markingListModel
               READ markingListModel CONSTANT)
    Q_PROPERTY(viewmodels::WarningListModel* warningListModel
               READ warningListModel CONSTANT)

    // Глобальное состояние подключения
    Q_PROPERTY(bool isFullyConnected READ isFullyConnected
               NOTIFY connectionStateChanged)
    Q_PROPERTY(bool isDataConnected READ isDataConnected
               NOTIFY dataConnectionChanged)
    Q_PROPERTY(bool isVideoConnected READ isVideoConnected
               NOTIFY videoConnectionChanged)

    // Статус сообщение для UI
    Q_PROPERTY(QString statusMessage READ statusMessage
               NOTIFY statusMessageChanged)

    // Sync monitor
    Q_PROPERTY(SynchronizationMonitor* syncMonitor
               READ syncMonitor CONSTANT)

public:
    explicit AppController(QObject* parent = nullptr);
    ~AppController() override;

    // === Lifecycle ===

    // Инициализация с путем к конфигурации
    bool initialize(const QString& config_path = "config.json");

    // Graceful shutdown
    void shutdown();

    // === Subsystem Access ===

    network::ConnectionManager* connectionManager() const
        { return connection_manager_; }

    video::NetworkVideoWidget* videoWidget() const
        { return video_widget_; }

    SynchronizationMonitor* syncMonitor() const
        { return sync_monitor_; }

    // === ViewModel Access ===

    viewmodels::LaneStateViewModel* laneViewModel() const;
    viewmodels::MarkingObjectListModel* markingListModel() const;
    viewmodels::WarningListModel* warningListModel() const;

    // === State Getters ===

    bool isFullyConnected() const { return is_fully_connected_; }
    bool isDataConnected() const { return is_data_connected_; }
    bool isVideoConnected() const { return is_video_connected_; }
    QString statusMessage() const { return status_message_; }

    // === Configuration ===

    const config::AppConfig& config() const { return config_; }

signals:
    // Lifecycle events
    void initializationComplete();
    void shutdownComplete();

    // Connection state changes
    void connectionStateChanged(bool fully_connected);
    void dataConnectionChanged(bool connected);
    void videoConnectionChanged(bool connected);

    // Status updates
    void statusMessageChanged(const QString& message);

    // Critical errors
    void criticalError(const QString& error);

private:
    // === Core Subsystems (owned) ===
    network::ConnectionManager* connection_manager_{nullptr};
    video::NetworkVideoWidget* video_widget_{nullptr};
    video::MarkingOverlayProcessor* overlay_processor_{nullptr};
    SynchronizationMonitor* sync_monitor_{nullptr};

    // === Configuration ===
    config::AppConfig config_;

    // === State ===
    bool is_fully_connected_{false};
    bool is_data_connected_{false};
    bool is_video_connected_{false};
    QString status_message_{"Not initialized"};

    // === Setup Methods ===

    // Создание всех компонентов
    void createComponents();

    // Конфигурация компонентов из config_
    void configureComponents();

    // Связывание компонентов (signal/slot connections)
    void wireComponents();

    // === State Management ===

    void updateGlobalConnectionState();
    void updateStatusMessage(const QString& message);
    void setDataConnected(bool connected);
    void setVideoConnected(bool connected);

private slots:
    // === Data Flow Handlers ===

    // ConnectionManager → Overlay
    void onMarkingModelUpdated();
    void onLaneStateUpdated();

    // === Connection State Handlers ===

    void onDataConnectionStateChanged(network::ConnectionManager::State state);
    void onVideoConnectionStateChanged(bool connected);

    // === Error Handlers ===

    void onDataConnectionError(const QString& error);
    void onVideoConnectionError(const QString& error);
};

} // namespace app
```

**Детальная логика методов**:

#### initialize()

```cpp
bool AppController::initialize(const QString& config_path) {
    LOG_INFO << "Initializing AppController with config: "
             << config_path.toStdString();

    // 1. Load configuration
    try {
        config_ = config::ConfigurationManager::loadFromFile(config_path);
        LOG_INFO << "Configuration loaded successfully";
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to load config: " << e.what();
        LOG_WARN << "Using default configuration";
        config_ = config::ConfigurationManager::defaultConfig();
    }

    // 2. Create all components (eager initialization)
    try {
        createComponents();
        LOG_INFO << "All components created";
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to create components: " << e.what();
        updateStatusMessage("Initialization failed: components creation");
        emit criticalError(QString("Component creation failed: %1").arg(e.what()));
        return false;
    }

    // 3. Configure components with loaded config
    try {
        configureComponents();
        LOG_INFO << "All components configured";
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to configure components: " << e.what();
        updateStatusMessage("Initialization failed: configuration");
        emit criticalError(QString("Component configuration failed: %1").arg(e.what()));
        return false;
    }

    // 4. Wire components together (signal/slot connections)
    try {
        wireComponents();
        LOG_INFO << "All components wired";
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to wire components: " << e.what();
        updateStatusMessage("Initialization failed: wiring");
        emit criticalError(QString("Component wiring failed: %1").arg(e.what()));
        return false;
    }

    updateStatusMessage("Initialized, ready to connect");
    emit initializationComplete();

    LOG_INFO << "AppController initialization complete";
    return true;
}
```

#### createComponents()

```cpp
void AppController::createComponents() {
    // Network layer
    connection_manager_ = new network::ConnectionManager(this);
    LOG_DEBUG << "ConnectionManager created";

    // Video layer - НЕ ставим parent, т.к. это QWidget
    video_widget_ = new video::NetworkVideoWidget(nullptr);
    LOG_DEBUG << "NetworkVideoWidget created";

    // Overlay processor
    overlay_processor_ = new video::MarkingOverlayProcessor(this);
    LOG_DEBUG << "MarkingOverlayProcessor created";

    // Sync monitor (создаем с временным порогом, настроим в configure)
    sync_monitor_ = new SynchronizationMonitor(500, this);
    LOG_DEBUG << "SynchronizationMonitor created";
}
```

#### configureComponents()

```cpp
void AppController::configureComponents() {
    // === Configure ConnectionManager ===
    connection_manager_->setAutoReconnect(config_.network.auto_reconnect);
    connection_manager_->setReconnectInterval(config_.network.reconnect_interval_ms);
    connection_manager_->setMaxReconnectAttempts(config_.network.max_reconnect_attempts);
    LOG_DEBUG << "ConnectionManager configured: host="
              << config_.network.host.toStdString()
              << " port=" << config_.network.port;

    // === Configure WarningEngine ===
    // ВАЖНО: Нужно добавить метод в ConnectionManager:
    // void setWarningEngineConfig(const domain::WarningEngineConfig& config);

    auto warning_config = config_.warning.toDomainConfig();
    // connection_manager_->setWarningEngineConfig(warning_config);
    // TODO: Реализовать этот метод в ConnectionManager
    LOG_DEBUG << "WarningEngine configuration prepared";

    // === Configure VideoWidget ===
    video_widget_->setSourceUrl(config_.video.source_url);
    video_widget_->setAutoStart(config_.video.auto_start);
    LOG_DEBUG << "VideoWidget configured: url="
              << config_.video.source_url.toStdString();

    // === Add overlay processor to video widget ===
    video_widget_->addProcessor(overlay_processor_);
    LOG_DEBUG << "MarkingOverlayProcessor added to VideoWidget";

    // === Configure SyncMonitor ===
    // Пересоздаем с правильным порогом из конфига
    delete sync_monitor_;
    sync_monitor_ = new SynchronizationMonitor(
        config_.sync.max_timestamp_diff_ms, this);
    LOG_DEBUG << "SyncMonitor configured with threshold="
              << config_.sync.max_timestamp_diff_ms << "ms";
}
```

#### wireComponents()

```cpp
void AppController::wireComponents() {
    // === ConnectionManager → Data Updates ===

    connect(connection_manager_,
            &network::ConnectionManager::laneStateUpdated,
            this, &AppController::onLaneStateUpdated);

    connect(connection_manager_,
            &network::ConnectionManager::markingModelUpdated,
            this, &AppController::onMarkingModelUpdated);

    connect(connection_manager_,
            &network::ConnectionManager::warningModelUpdated,
            this, [this]() {
                LOG_DEBUG << "Warning model updated";
            });

    // === ConnectionManager → State Changes ===

    connect(connection_manager_,
            &network::ConnectionManager::stateChanged,
            this, &AppController::onDataConnectionStateChanged);

    connect(connection_manager_,
            &network::ConnectionManager::lastErrorChanged,
            this, &AppController::onDataConnectionError);

    // === VideoWidget → State Changes ===

    connect(video_widget_,
            &video::NetworkVideoWidget::connectedChanged,
            this, &AppController::onVideoConnectionStateChanged);

    connect(video_widget_,
            &video::NetworkVideoWidget::connectionFailed,
            this, &AppController::onVideoConnectionError);

    // === SyncMonitor → Warnings ===

    connect(sync_monitor_,
            &SynchronizationMonitor::desyncWarning,
            this, [this](const QString& msg) {
                LOG_WARN << "Desync warning: " << msg.toStdString();
                // Можно показать в UI через statusMessage
                updateStatusMessage("Warning: " + msg);
            });

    connect(sync_monitor_,
            &SynchronizationMonitor::syncRestored,
            this, [this]() {
                LOG_INFO << "Synchronization restored";
            });

    LOG_DEBUG << "All components wired successfully";
}
```

#### Обработчики событий

```cpp
void AppController::onLaneStateUpdated() {
    const auto& lane_state = connection_manager_->laneState();

    // Update sync monitor with data timestamp
    if (config_.sync.enable_sync_monitoring) {
        sync_monitor_->updateDataTimestamp(lane_state.timestampMs());
    }

    LOG_DEBUG << "Lane state updated, ts=" << lane_state.timestampMs();
}

void AppController::onMarkingModelUpdated() {
    const auto& marking_model = connection_manager_->markingModel();

    // Update video overlay with latest marking data
    overlay_processor_->updateMarkings(marking_model);

    LOG_DEBUG << "Marking model updated, count=" << marking_model.size();
}

void AppController::onDataConnectionStateChanged(
    network::ConnectionManager::State state)
{
    bool connected = (state == network::ConnectionManager::State::Connected);
    setDataConnected(connected);

    QString state_str;
    switch (state) {
        case network::ConnectionManager::State::Connected:
            state_str = "Data connected";
            break;
        case network::ConnectionManager::State::Disconnected:
            state_str = "Data disconnected";
            break;
        case network::ConnectionManager::State::Connecting:
            state_str = "Connecting to data...";
            break;
        case network::ConnectionManager::State::Reconnecting:
            state_str = "Reconnecting to data...";
            break;
        case network::ConnectionManager::State::Error:
            state_str = "Data connection error";
            break;
        default:
            state_str = "Data state unknown";
    }

    updateStatusMessage(state_str);
    LOG_INFO << "Data connection state: " << state_str.toStdString();
}

void AppController::onVideoConnectionStateChanged(bool connected) {
    setVideoConnected(connected);
    updateStatusMessage(connected ? "Video connected" : "Video disconnected");
    LOG_INFO << "Video connection: " << (connected ? "YES" : "NO");
}

void AppController::setDataConnected(bool connected) {
    if (is_data_connected_ != connected) {
        is_data_connected_ = connected;
        emit dataConnectionChanged(connected);
        updateGlobalConnectionState();
    }
}

void AppController::setVideoConnected(bool connected) {
    if (is_video_connected_ != connected) {
        is_video_connected_ = connected;
        emit videoConnectionChanged(connected);
        updateGlobalConnectionState();
    }
}

void AppController::updateGlobalConnectionState() {
    bool new_state = is_data_connected_ && is_video_connected_;

    if (is_fully_connected_ != new_state) {
        is_fully_connected_ = new_state;
        emit connectionStateChanged(is_fully_connected_);

        if (is_fully_connected_) {
            LOG_INFO << "System FULLY connected (data + video)";
            updateStatusMessage("Fully connected");
        } else {
            QString status = QString("Partial: Data=%1 Video=%2")
                .arg(is_data_connected_ ? "OK" : "NO")
                .arg(is_video_connected_ ? "OK" : "NO");
            LOG_WARN << status.toStdString();
            updateStatusMessage(status);
        }
    }
}

void AppController::updateStatusMessage(const QString& message) {
    if (status_message_ != message) {
        status_message_ = message;
        emit statusMessageChanged(status_message_);
    }
}
```

---

### 4. MainWindow

**Файл**: `ui/MainWindow.hpp`, `ui/MainWindow.cpp`

**Назначение**: Главное окно приложения (UI layer)

**API**:

```cpp
namespace ui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(app::AppController* controller, QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    app::AppController* controller_;

    // UI Components
    QWidget* central_widget_;
    QVBoxLayout* main_layout_;

    // Status bar
    QLabel* status_label_;
    QLabel* data_status_label_;
    QLabel* video_status_label_;
    QLabel* sync_status_label_;

    // Panels
    QWidget* video_container_;
    QWidget* lane_info_panel_;
    QWidget* marking_list_panel_;
    QWidget* warning_list_panel_;

    // QML containers (опционально)
    QQuickWidget* qml_video_panel_{nullptr};
    QQuickWidget* qml_lane_panel_{nullptr};

    void setupUi();
    void setupMenuBar();
    void setupStatusBar();
    void setupPanels();
    void setupConnections();

    // Actions
    void onConnectAction();
    void onDisconnectAction();
    void onAboutAction();
};

} // namespace ui
```

**Реализация**:

```cpp
MainWindow::MainWindow(app::AppController* controller, QWidget* parent)
    : QMainWindow(parent)
    , controller_(controller)
{
    setupUi();
    setupConnections();
}

void MainWindow::setupUi() {
    setWindowTitle("Dashboard - Road Marking Detector");
    resize(1280, 720);

    central_widget_ = new QWidget(this);
    setCentralWidget(central_widget_);

    main_layout_ = new QVBoxLayout(central_widget_);

    setupMenuBar();
    setupStatusBar();
    setupPanels();
}

void MainWindow::setupPanels() {
    // Video panel
    video_container_ = new QWidget(this);
    video_container_->setMinimumSize(640, 480);

    // Встраиваем video widget из AppController
    auto* video_layout = new QVBoxLayout(video_container_);
    video_layout->addWidget(controller_->videoWidget());

    main_layout_->addWidget(video_container_);

    // Остальные панели...
}

void MainWindow::setupConnections() {
    // AppController → UI updates
    connect(controller_, &app::AppController::statusMessageChanged,
            this, [this](const QString& msg) {
                status_label_->setText(msg);
            });

    connect(controller_, &app::AppController::connectionStateChanged,
            this, [this](bool connected) {
                // Обновить индикаторы
            });

    // И т.д.
}

void MainWindow::closeEvent(QCloseEvent* event) {
    controller_->shutdown();
    event->accept();
}
```

---

### 5. Модификации существующих компонентов

#### ConnectionManager

**Нужно добавить**:

```cpp
// В ConnectionManager.h
public:
    // Доступ к WarningEngine для конфигурации
    domain::WarningEngine* warningEngine() { return &warning_engine_; }
    const domain::WarningEngine* warningEngine() const { return &warning_engine_; }

    // Метод для установки конфигурации WarningEngine
    void setWarningEngineConfig(const domain::WarningEngineConfig& config) {
        warning_engine_.setConfig(config);
        LOG_INFO << "WarningEngine configuration updated";
    }
```

#### NetworkVideoWidget

**Нужно добавить**:

```cpp
// В NetworkVideoWidget.hpp
signals:
    // Сигнал при отображении нового кадра с timestamp
    void frameDisplayed(quint64 timestamp_ms);

// В NetworkVideoWidget.cpp
// При отрисовке кадра:
void NetworkVideoWidget::paintEvent(QPaintEvent* event) {
    // ... existing code ...

    // Emit signal с timestamp текущего кадра
    emit frameDisplayed(current_frame_timestamp_ms_);
}
```

---

## Потоки данных

### 1. Поток данных от сети

```
TCP Server
    ↓
TcpReaderWorker (parse protobuf)
    ↓
ConnectionManager::laneSummaryReceived()
    ↓
├─→ domain::LaneState::updateFromProto()
│       ↓
│   viewmodels::LaneStateViewModel::updateFromDomain()
│       ↓
│   UI (QML/Widgets) через bindings
│
├─→ domain::WarningEngine::update()
│       ↓
│   domain::WarningModel
│       ↓
│   viewmodels::WarningListModel::updateFromDomain()
│       ↓
│   UI
│
└─→ emit ConnectionManager::laneStateUpdated()
        ↓
    AppController::onLaneStateUpdated()
        ↓
    SynchronizationMonitor::updateDataTimestamp()
```

### 2. Поток видео данных

```
RTSP Stream
    ↓
QtMultimediaVideoProvider
    ↓
NetworkVideoWidget::paintEvent()
    ↓
├─→ MarkingOverlayProcessor::processFrame()
│   (рисует разметку)
│       ↓
│   QImage с наложенной разметкой
│       ↓
│   Отображение в VideoWidget
│
└─→ emit NetworkVideoWidget::frameDisplayed(timestamp)
        ↓
    AppController (если подключен)
        ↓
    SynchronizationMonitor::updateVideoTimestamp()
```

### 3. Поток разметки на видео

```
ConnectionManager::markingModelUpdated()
    ↓
AppController::onMarkingModelUpdated()
    ↓
MarkingOverlayProcessor::updateMarkings(model)
    ↓
Хранит текущую marking model
    ↓
При следующем processFrame() рисует актуальную разметку
```

---

## Конфигурация

### Структура config.json

```json
{
  "network": {
    "host": "192.168.1.100",
    "port": 5000,
    "reconnect_interval_ms": 5000,
    "max_reconnect_attempts": 0,
    "auto_reconnect": true
  },
  "video": {
    "source_url": "rtsp://192.168.1.100:8554/stream",
    "auto_start": false
  },
  "warning": {
    "lane_departure_threshold_m": 0.3,
    "crosswalk_distance_threshold_m": 30.0,
    "crosswalk_critical_distance_m": 10.0,
    "min_marking_confidence": 50,
    "min_lane_quality": 60,
    "enable_crosswalk_warnings": true,
    "enable_lane_departure_warnings": true
  },
  "sync": {
    "max_timestamp_diff_ms": 500,
    "enable_sync_monitoring": true
  }
}
```

### Расположение файла

**Приоритет поиска**:
1. Путь указанный в аргументе `initialize(path)`
2. `./config.json` (рядом с исполняемым файлом)
3. `~/.config/dashboard/config.json` (Linux)
4. `%APPDATA%/Dashboard/config.json` (Windows)
5. Default config (hardcoded)

---

## План реализации

### Фаза 1: Инфраструктура (config, app layer)

#### Задача 1.1: Создать структуру директорий
- [ ] Создать `app/`
- [ ] Создать `config/`
- [ ] Создать `ui/`
- [ ] Создать `ui/qml/`

#### Задача 1.2: ConfigurationManager
- [ ] Создать `config/AppConfig.hpp` со всеми структурами
- [ ] Реализовать сериализацию/десериализацию JSON
- [ ] Реализовать `ConfigurationManager::loadFromFile()`
- [ ] Реализовать `ConfigurationManager::saveToFile()`
- [ ] Реализовать валидацию
- [ ] Написать unit тесты
- [ ] Создать пример `config.json`

#### Задача 1.3: SynchronizationMonitor
- [ ] Создать `app/SynchronizationMonitor.hpp`
- [ ] Реализовать логику мониторинга timestamp
- [ ] Реализовать генерацию warnings
- [ ] Добавить Q_PROPERTY для QML
- [ ] Написать unit тесты

#### Задача 1.4: AppController (базовая версия)
- [ ] Создать `app/AppController.hpp`
- [ ] Реализовать `initialize()`
- [ ] Реализовать `createComponents()`
- [ ] Реализовать `configureComponents()`
- [ ] Реализовать `wireComponents()`
- [ ] Реализовать `shutdown()`
- [ ] Добавить обработчики событий

---

### Фаза 2: Модификация существующих компонентов

#### Задача 2.1: Расширить ConnectionManager
- [ ] Добавить метод `setWarningEngineConfig()`
- [ ] Добавить getter для `warningEngine()`
- [ ] Протестировать изменения

#### Задача 2.2: Расширить NetworkVideoWidget
- [ ] Добавить signal `frameDisplayed(quint64 timestamp)`
- [ ] Реализовать эмит этого сигнала в `paintEvent()`
- [ ] Убедиться что timestamp корректный
- [ ] Протестировать

#### Задача 2.3: Проверить MarkingOverlayProcessor
- [ ] Убедиться что есть метод `updateMarkings()`
- [ ] Если нет - реализовать
- [ ] Протестировать наложение разметки

---

### Фаза 3: UI Layer

#### Задача 3.1: MainWindow (базовая версия)
- [ ] Создать `ui/MainWindow.hpp/cpp`
- [ ] Реализовать базовый layout
- [ ] Встроить VideoWidget из AppController
- [ ] Создать status bar
- [ ] Создать menu bar

#### Задача 3.2: Панели информации (Widgets)
- [ ] Создать панель LaneState (Qt Widgets)
- [ ] Создать панель Warnings (Qt Widgets)
- [ ] Создать панель Marking Objects (Qt Widgets)
- [ ] Подключить к ViewModels

#### Задача 3.3: QML компоненты (опционально)
- [ ] Создать `ui/qml/VideoPanel.qml`
- [ ] Создать `ui/qml/LaneStatePanel.qml`
- [ ] Создать `ui/qml/WarningListPanel.qml`
- [ ] Зарегистрировать ViewModels в QML context
- [ ] Интегрировать через QQuickWidget

---

### Фаза 4: Интеграция и тестирование

#### Задача 4.1: Модифицировать main.cpp
- [ ] Убрать старый код
- [ ] Добавить создание AppController
- [ ] Добавить создание MainWindow
- [ ] Добавить обработку ошибок инициализации
- [ ] Добавить graceful shutdown

#### Задача 4.2: Обновить CMakeLists.txt
- [ ] Добавить новые директории в include_directories
- [ ] Убедиться что все новые .cpp файлы включены
- [ ] Добавить Qt JSON если нужно
- [ ] Проверить сборку

#### Задача 4.3: Интеграционное тестирование
- [ ] Тест: загрузка конфигурации
- [ ] Тест: инициализация всех компонентов
- [ ] Тест: подключение к данным
- [ ] Тест: подключение к видео
- [ ] Тест: передача данных через AppController
- [ ] Тест: синхронизация timestamp
- [ ] Тест: graceful shutdown

#### Задача 4.4: End-to-end тестирование
- [ ] Запуск с реальным сервером данных
- [ ] Запуск с реальным видеопотоком
- [ ] Проверка всех панелей UI
- [ ] Проверка warnings
- [ ] Проверка overlay на видео
- [ ] Проверка мониторинга синхронизации

---

### Фаза 5: Полировка и документация

#### Задача 5.1: Обработка ошибок
- [ ] Добавить обработку всех исключений
- [ ] Добавить user-friendly сообщения об ошибках
- [ ] Добавить fallback на default config при ошибках

#### Задача 5.2: Логирование
- [ ] Добавить логи во все критические места
- [ ] Настроить уровни логирования
- [ ] Убедиться что логи информативные

#### Задача 5.3: Документация
- [ ] Документировать API всех новых классов
- [ ] Создать README.md с инструкциями по сборке
- [ ] Создать USER_GUIDE.md
- [ ] Задокументировать формат config.json

---

## Контракты и интерфейсы

### AppController → ConnectionManager

**Используемые методы**:
```cpp
// State
bool isConnected() const;
State state() const;

// Control
void connectToHost(const QString& host, int port);
void disconnectFromHost();

// Configuration
void setWarningEngineConfig(const domain::WarningEngineConfig& config);

// Data access
const domain::LaneState& laneState() const;
const domain::MarkingObjectModel& markingModel() const;
const domain::WarningModel& warningModel() const;

// ViewModels
viewmodels::LaneStateViewModel* laneViewModel() const;
viewmodels::MarkingObjectListModel* markingListModel() const;
viewmodels::WarningListModel* warningListModel() const;
```

**Подписка на сигналы**:
```cpp
void stateChanged(State state);
void laneStateUpdated();
void markingModelUpdated();
void warningModelUpdated();
void lastErrorChanged(const QString& error);
```

---

### AppController → NetworkVideoWidget

**Используемые методы**:
```cpp
// Configuration
void setSourceUrl(const QString& url);
void setAutoStart(bool enabled);

// Control
void connectToSource();
void disconnectFromSource();

// State
bool isConnected() const;

// Processor management
void addProcessor(IVideoFrameProcessor* processor);
```

**Подписка на сигналы**:
```cpp
void connectedChanged(bool connected);
void connectionFailed(const QString& error);
void frameDisplayed(quint64 timestamp_ms); // [НОВЫЙ]
```

---

### AppController → MarkingOverlayProcessor

**Используемые методы**:
```cpp
// Update marking data
void updateMarkings(const domain::MarkingObjectModel& model);
```

---

### AppController → SynchronizationMonitor

**Используемые методы**:
```cpp
// Update timestamps
void updateDataTimestamp(std::uint64_t timestamp_ms);
void updateVideoTimestamp(std::uint64_t timestamp_ms);

// State
int timestampDiffMs() const;
bool isSynchronized() const;
```

**Подписка на сигналы**:
```cpp
void timestampDiffChanged(int diff_ms);
void synchronizationChanged(bool synced);
void desyncWarning(const QString& message);
void syncRestored();
```

---

### MainWindow → AppController

**Используемые методы**:
```cpp
// Subsystem access
network::ConnectionManager* connectionManager() const;
video::NetworkVideoWidget* videoWidget() const;
SynchronizationMonitor* syncMonitor() const;

// ViewModel access
viewmodels::LaneStateViewModel* laneViewModel() const;
viewmodels::MarkingObjectListModel* markingListModel() const;
viewmodels::WarningListModel* warningListModel() const;

// State
bool isFullyConnected() const;
bool isDataConnected() const;
bool isVideoConnected() const;
QString statusMessage() const;

// Control
void shutdown();
```

**Подписка на сигналы**:
```cpp
void connectionStateChanged(bool fully_connected);
void dataConnectionChanged(bool connected);
void videoConnectionChanged(bool connected);
void statusMessageChanged(const QString& message);
void criticalError(const QString& error);
```

---

## Дополнительные замечания

### Управление памятью

1. **AppController** владеет:
   - `ConnectionManager` (parent = AppController)
   - `MarkingOverlayProcessor` (parent = AppController)
   - `SynchronizationMonitor` (parent = AppController)
   - `NetworkVideoWidget` (**НЕТ** parent, удаляется вручную в деструкторе)

2. **MainWindow** НЕ владеет ничем, только использует указатели

3. **ViewModels** создаются и владеются `ConnectionManager`

### Потокобезопасность

- **ConnectionManager** уже использует `TcpReaderWorker` в отдельном потоке
- **AppController** работает в главном потоке Qt
- **Все signal/slot соединения** между потоками автоматически queued Qt
- **Синхронизация** не требуется, т.к. все обновления через signal/slot

### Расширяемость

Архитектура позволяет легко добавить:
- Несколько источников данных (создать несколько ConnectionManager)
- Запись видео (добавить processor)
- Статистику (добавить в AppController)
- Удаленное управление (добавить API в AppController)
- Плагины (реализовать plugin system в AppController)

---

## Чек-лист готовности к реализации

- [x] Архитектура спроектирована
- [x] Компоненты определены
- [x] API специфицирован
- [x] Потоки данных определены
- [x] План реализации составлен
- [ ] Структура директорий создана
- [ ] CMakeLists.txt обновлен
- [ ] Можно начинать реализацию

---

**Следующий шаг**: Начать с Фазы 1 - создание инфраструктуры (ConfigurationManager и SynchronizationMonitor)
