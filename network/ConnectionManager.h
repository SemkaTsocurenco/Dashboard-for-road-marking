#pragma once 

#include <QObject>
#include <QString>
#include "TcpReaderWorker.h"
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

    signals:
        void lastErrorChanged(const QString& error);
        void connectedChanged(bool connected);
        void stateChanged(State state);

    private: 
        void setState(State newState);
        void setConnected(bool connected);
        void setLastError(const QString& error);

        void createWorkerIfNeeded();
        void destroyWorker();

        State state_{State::Disconnected};
        bool connected_{false};
        QString last_error_;
        
        QThread* workerThread_{nullptr};
        TcpReaderWorker* worker_{nullptr};
    };
}