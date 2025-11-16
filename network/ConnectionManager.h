#pragma once 

#include <QObject>
#include <QString>
#include <QTimer>
#include "TcpReaderWorker.h"
#include "proto_parser.h"
#include <QThread>

namespace network {

    class ConnectionManager : public QObject 
    {
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

    signals:
        void lastErrorChanged(const QString& error);
        void connectedChanged(bool connected);
        void stateChanged(State state);
        void laneSummaryReceived(const laneproto::LaneSummary&);
        void markingObjectsReceived(const laneproto::MarkingObjects&);
        void reconnectAttempt(int attempt, int maxAttempts);
        // void warningsReceived(const Warnings&);


    private:
        void setState(State newState);
        void setConnected(bool connected);
        void setLastError(const QString& error);

        void createWorkerIfNeeded();
        void destroyWorker();

        void scheduleReconnect();
        void attemptReconnect();
        void resetReconnectState();

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
    };
}