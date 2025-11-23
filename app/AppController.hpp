#pragma once

#include <QObject>
#include "AppConfig.hpp"
#include "ConnectionManager.h"
#include "NetworkVideoWidget.hpp"
#include "MarkingOverlayProcessor.hpp"
#include "SynchronizationMonitor.hpp"

namespace app {

/**
 * @brief Central application controller that coordinates all subsystems
 */
class AppController : public QObject {
    Q_OBJECT

    //  ViewModels for direct access from QML/UI 
    Q_PROPERTY(viewmodels::LaneStateViewModel* laneViewModel
               READ laneViewModel CONSTANT)
    Q_PROPERTY(viewmodels::MarkingObjectListModel* markingListModel
               READ markingListModel CONSTANT)
    Q_PROPERTY(viewmodels::WarningListModel* warningListModel
               READ warningListModel CONSTANT)

    //  Global connection state 
    Q_PROPERTY(bool isFullyConnected READ isFullyConnected
               NOTIFY connectionStateChanged)
    Q_PROPERTY(bool isDataConnected READ isDataConnected
               NOTIFY dataConnectionChanged)
    Q_PROPERTY(bool isVideoConnected READ isVideoConnected
               NOTIFY videoConnectionChanged)

    //  Status message for UI 
    Q_PROPERTY(QString statusMessage READ statusMessage
               NOTIFY statusMessageChanged)

    //  Sync monitor access 
    Q_PROPERTY(SynchronizationMonitor* syncMonitor
               READ syncMonitor CONSTANT)

public:
    explicit AppController(QObject* parent = nullptr);
    ~AppController() override;

    bool initialize(const QString& config_path = "config.json");
    void shutdown();

    network::ConnectionManager* connectionManager() const
        { return connection_manager_; }

    video::NetworkVideoWidget* videoWidget() const
        { return video_widget_; }

    SynchronizationMonitor* syncMonitor() const
        { return sync_monitor_; }


    viewmodels::LaneStateViewModel* laneViewModel() const;
    viewmodels::MarkingObjectListModel* markingListModel() const;
    viewmodels::WarningListModel* warningListModel() const;

    bool isFullyConnected() const { return is_fully_connected_; }
    bool isDataConnected() const { return is_data_connected_; }
    bool isVideoConnected() const { return is_video_connected_; }
    QString statusMessage() const { return status_message_; }

    const config::AppConfig& config() const { return config_; }

signals:
    void initializationComplete();
    void shutdownComplete();

    void connectionStateChanged(bool fully_connected);
    void dataConnectionChanged(bool connected);
    void videoConnectionChanged(bool connected);

    void statusMessageChanged(const QString& message);

    void criticalError(const QString& error);

private:
    network::ConnectionManager* connection_manager_{nullptr};
    video::NetworkVideoWidget* video_widget_{nullptr};
    video::FrameProcessorPtr overlay_processor_{nullptr};
    SynchronizationMonitor* sync_monitor_{nullptr};

    config::AppConfig config_;

    bool is_fully_connected_{false};
    bool is_data_connected_{false};
    bool is_video_connected_{false};
    QString status_message_{"Not initialized"};

    void createComponents();
    void configureComponents();
    void wireComponents();

    void updateGlobalConnectionState();
    void updateStatusMessage(const QString& message);
    void setDataConnected(bool connected);
    void setVideoConnected(bool connected);

private slots:

    void onMarkingModelUpdated();
    void onLaneStateUpdated();

    void onDataConnectionStateChanged(network::ConnectionManager::State state);
    void onVideoConnectionStateChanged(bool connected);

    void onDataConnectionError(const QString& error);
    void onVideoConnectionError(const QString& error);
};

} // namespace app
