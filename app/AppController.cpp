#include "AppController.hpp"
#include "ConfigurationManager.hpp"
#include "LoggerMacros.hpp"

namespace app {

AppController::AppController(QObject* parent)
    : QObject(parent)
{
    LOG_INFO << "AppController created";
}

AppController::~AppController()
{
    LOG_INFO << "AppController destroying";
    shutdown();

    if (video_widget_) {
        delete video_widget_;
        video_widget_ = nullptr;
    }
}


bool AppController::initialize(const QString& config_path)
{
    LOG_INFO << "Initializing AppController with config: " << config_path.toStdString();

    try {
        config_ = config::ConfigurationManager::loadFromFile(config_path);
        LOG_INFO << "Configuration loaded successfully";
    } catch (const config::ConfigurationException& e) {
        LOG_ERROR << "Failed to load config: " << e.what();
        LOG_WARN << "Using default configuration";
        config_ = config::ConfigurationManager::defaultConfig();
    } catch (const std::exception& e) {
        LOG_ERROR << "Unexpected error loading config: " << e.what();
        LOG_WARN << "Using default configuration";
        config_ = config::ConfigurationManager::defaultConfig();
    }

    try {
        createComponents();
        LOG_INFO << "All components created";
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to create components: " << e.what();
        updateStatusMessage("Initialization failed: components creation");
        emit criticalError(QString("Component creation failed: %1").arg(e.what()));
        return false;
    }
    try {
        configureComponents();
        LOG_INFO << "All components configured";
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to configure components: " << e.what();
        updateStatusMessage("Initialization failed: configuration");
        emit criticalError(QString("Component configuration failed: %1").arg(e.what()));
        return false;
    }

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

void AppController::shutdown()
{
    LOG_INFO << "Shutting down AppController";

    if (connection_manager_) {
        connection_manager_->disconnectFromHost();
    }

    if (video_widget_) {
        video_widget_->disconnectFromSource();
    }

    updateStatusMessage("Shutdown complete");
    emit shutdownComplete();
}


void AppController::createComponents()
{
    LOG_DEBUG << "Creating components...";

    connection_manager_ = new network::ConnectionManager(this);
    LOG_DEBUG << "ConnectionManager created";

    video_widget_ = new video::NetworkVideoWidget(nullptr);
    LOG_DEBUG << "NetworkVideoWidget created";

    overlay_processor_ = video::FrameProcessorPtr(new video::MarkingOverlayProcessor());
    LOG_DEBUG << "MarkingOverlayProcessor created";

    sync_monitor_ = new SynchronizationMonitor(500, this);
    LOG_DEBUG << "SynchronizationMonitor created";
}


void AppController::configureComponents()
{
    LOG_DEBUG << "Configuring components...";

    connection_manager_->setAutoReconnect(config_.network.auto_reconnect);
    connection_manager_->setReconnectInterval(config_.network.reconnect_interval_ms);
    connection_manager_->setMaxReconnectAttempts(config_.network.max_reconnect_attempts);
    LOG_DEBUG << "ConnectionManager configured: host=" << config_.network.host.toStdString()
              << " port=" << config_.network.port;

    domain::WarningEngineConfig warning_config = config_.warning.toDomainConfig();
    connection_manager_->setWarningEngineConfig(warning_config);
    LOG_DEBUG << "WarningEngine configured";

    video_widget_->setSourceUrl(config_.video.source_url);
    video_widget_->setAutoStart(config_.video.auto_start);
    LOG_DEBUG << "VideoWidget configured: url=" << config_.video.source_url.toStdString();

    video_widget_->addFrameProcessor(overlay_processor_);

    auto* marking_processor = dynamic_cast<video::MarkingOverlayProcessor*>(overlay_processor_.data());
    if (marking_processor) {
        marking_processor->setMarkingObjectListModel(connection_manager_->markingListModel());
        marking_processor->setLaneStateViewModel(connection_manager_->laneViewModel());
        marking_processor->setWarningListModel(connection_manager_->warningListModel());
        LOG_DEBUG << "MarkingOverlayProcessor configured with ViewModels";
    }
    LOG_DEBUG << "MarkingOverlayProcessor added to VideoWidget";

    if (sync_monitor_) {
        delete sync_monitor_;
    }
    sync_monitor_ = new SynchronizationMonitor(config_.sync.max_timestamp_diff_ms, this);
    LOG_DEBUG << "SyncMonitor configured with threshold=" << config_.sync.max_timestamp_diff_ms << "ms";
}


void AppController::wireComponents()
{
    LOG_DEBUG << "Wiring components...";

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

    connect(connection_manager_,
            &network::ConnectionManager::stateChanged,
            this, &AppController::onDataConnectionStateChanged);

    connect(connection_manager_,
            &network::ConnectionManager::lastErrorChanged,
            this, &AppController::onDataConnectionError);

    connect(video_widget_,
            &video::NetworkVideoWidget::connectedChanged,
            this, &AppController::onVideoConnectionStateChanged);

    connect(video_widget_,
            &video::NetworkVideoWidget::connectionFailed,
            this, &AppController::onVideoConnectionError);

    if (config_.sync.enable_sync_monitoring) {
        connect(video_widget_,
                &video::NetworkVideoWidget::frameDisplayed,
                this, [this](quint64 timestamp_ms) {
                    sync_monitor_->updateVideoTimestamp(timestamp_ms);
                });
        LOG_DEBUG << "Video timestamp → SyncMonitor connected";
    }

    connect(sync_monitor_,
            &SynchronizationMonitor::desyncWarning,
            this, [this](const QString& msg) {
                LOG_WARN << "Desync warning: " << msg.toStdString();
                updateStatusMessage("Warning: " + msg);
            });

    connect(sync_monitor_,
            &SynchronizationMonitor::syncRestored,
            this, [this]() {
                LOG_INFO << "Synchronization restored";
            });

    LOG_DEBUG << "All components wired successfully";
}

viewmodels::LaneStateViewModel* AppController::laneViewModel() const
{
    return connection_manager_ ? connection_manager_->laneViewModel() : nullptr;
}

viewmodels::MarkingObjectListModel* AppController::markingListModel() const
{
    return connection_manager_ ? connection_manager_->markingListModel() : nullptr;
}

viewmodels::WarningListModel* AppController::warningListModel() const
{
    return connection_manager_ ? connection_manager_->warningListModel() : nullptr;
}

void AppController::onLaneStateUpdated()
{
    const auto& lane_state = connection_manager_->laneState();

    if (config_.sync.enable_sync_monitoring) {
        sync_monitor_->updateDataTimestamp(lane_state.timestampMs());
    }

    LOG_DEBUG << "Lane state updated, ts=" << lane_state.timestampMs();
}

void AppController::onMarkingModelUpdated()
{
    const auto& marking_model = connection_manager_->markingModel();

    auto* marking_processor = dynamic_cast<video::MarkingOverlayProcessor*>(overlay_processor_.data());
    if (marking_processor) {
        marking_processor->updateMarkings(marking_model);
    }

    LOG_DEBUG << "Marking model updated, count=" << marking_model.size();
}

void AppController::onDataConnectionStateChanged(network::ConnectionManager::State state)
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
        case network::ConnectionManager::State::Disconnecting:
            state_str = "Disconnecting from data...";
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

void AppController::onVideoConnectionStateChanged(bool connected)
{
    setVideoConnected(connected);
    updateStatusMessage(connected ? "Video connected" : "Video disconnected");
    LOG_INFO << "Video connection: " << (connected ? "YES" : "NO");
}

void AppController::onDataConnectionError(const QString& error)
{
    if (!error.isEmpty()) {
        LOG_ERROR << "Data connection error: " << error.toStdString();
        updateStatusMessage("Data error: " + error);
    }
}

void AppController::onVideoConnectionError(const QString& error)
{
    LOG_ERROR << "Video connection error: " << error.toStdString();
    updateStatusMessage("Video error: " + error);
}

void AppController::setDataConnected(bool connected)
{
    if (is_data_connected_ != connected) {
        is_data_connected_ = connected;
        emit dataConnectionChanged(connected);
        updateGlobalConnectionState();
    }
}

void AppController::setVideoConnected(bool connected)
{
    if (is_video_connected_ != connected) {
        is_video_connected_ = connected;
        emit videoConnectionChanged(connected);
        updateGlobalConnectionState();
    }
}

void AppController::updateGlobalConnectionState()
{
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

void AppController::updateStatusMessage(const QString& message)
{
    if (status_message_ != message) {
        status_message_ = message;
        emit statusMessageChanged(status_message_);
    }
}

} // namespace app
