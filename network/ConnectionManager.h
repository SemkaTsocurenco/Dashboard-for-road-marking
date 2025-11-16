#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QThread>
#include "TcpReaderWorker.h"
#include "proto_parser.h"
#include "MarkingObject.h"
#include "Warning.h"
#include "WarningEngine.h"
#include "LaneState.h"
#include "LaneStateViewModel.h"
#include "MarkingObjectListModel.h"
#include "WarningListModel.h"

namespace network {

    class ConnectionManager : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
        Q_PROPERTY(State state READ state NOTIFY stateChanged)
        Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
        Q_PROPERTY(bool autoReconnect READ autoReconnect WRITE setAutoReconnect)
        Q_PROPERTY(int reconnectInterval READ reconnectInterval WRITE setReconnectInterval)
        Q_PROPERTY(int maxReconnectAttempts READ maxReconnectAttempts WRITE setMaxReconnectAttempts)
        Q_PROPERTY(int currentReconnectAttempt READ currentReconnectAttempt NOTIFY reconnectAttempt)
        Q_PROPERTY(viewmodels::LaneStateViewModel* laneViewModel READ laneViewModel CONSTANT)
        Q_PROPERTY(viewmodels::MarkingObjectListModel* markingListModel READ markingListModel CONSTANT)
        Q_PROPERTY(viewmodels::WarningListModel* warningListModel READ warningListModel CONSTANT)

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

        explicit ConnectionManager(QObject* parent = nullptr);
        ~ConnectionManager() override;

        Q_INVOKABLE void connectToHost(const QString& host, int port);
        Q_INVOKABLE void disconnectFromHost();

        [[nodiscard]] bool isConnected() const;
        [[nodiscard]] State state() const;
        [[nodiscard]] QString lastError() const;

        Q_INVOKABLE void setAutoReconnect(bool enabled);
        [[nodiscard]] bool autoReconnect() const;

        Q_INVOKABLE void setReconnectInterval(int milliseconds);
        [[nodiscard]] int reconnectInterval() const;

        Q_INVOKABLE void setMaxReconnectAttempts(int attempts);
        [[nodiscard]] int maxReconnectAttempts() const;
        [[nodiscard]] int currentReconnectAttempt() const;

        const domain::LaneState& laneState() const noexcept { return lane_state_; }
        const domain::MarkingObjectModel& markingModel() const noexcept { return marking_model_; }
        const domain::WarningModel& warningModel() const noexcept { return warning_model_; }

        viewmodels::LaneStateViewModel* laneViewModel() const noexcept { return lane_view_model_; }
        viewmodels::MarkingObjectListModel* markingListModel() const noexcept { return marking_list_model_; }
        viewmodels::WarningListModel* warningListModel() const noexcept { return warning_list_model_; }


    signals:
        void lastErrorChanged(const QString& error);
        void connectedChanged(bool connected);
        void stateChanged(State state);
        void parseErrorReceived(const laneproto::ParseError& error);
        void reconnectAttempt(int attempt, int maxAttempts);
        void laneStateUpdated();
        void markingModelUpdated();
        void warningModelUpdated();


    private:
        void setState(State newState);
        void setConnected(bool connected);
        void setLastError(const QString& error);

        void createWorkerIfNeeded();
        void destroyWorker();

        void scheduleReconnect();
        void attemptReconnect();
        void resetReconnectState();

        // Protocol message handlers
        void laneSummaryReceived(const laneproto::LaneSummary& summary);
        void markingObjectsReceived(const laneproto::MarkingObjects& objects);

        void updateWarnings(std::uint64_t timestamp_ms);

        State state_{State::Disconnected};
        bool connected_{false};
        QString last_error_;

        QThread* workerThread_{nullptr};
        TcpReaderWorker* worker_{nullptr};

        bool auto_reconnect_{true};
        int reconnect_interval_{5000};  
        int max_reconnect_attempts_{0}; 
        int current_reconnect_attempt_{0};
        QString saved_host_;
        quint16 saved_port_{0};
        QTimer* reconnect_timer_{nullptr};

        domain::LaneState lane_state_;
        domain::MarkingObjectModel marking_model_;
        domain::WarningModel warning_model_;
        domain::WarningEngine warning_engine_;

        viewmodels::LaneStateViewModel* lane_view_model_{nullptr};
        viewmodels::MarkingObjectListModel* marking_list_model_{nullptr};
        viewmodels::WarningListModel* warning_list_model_{nullptr};
    };
}