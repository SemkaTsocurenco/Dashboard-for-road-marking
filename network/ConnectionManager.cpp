#include "ConnectionManager.h"
#include "LoggerMacros.hpp"
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <QTimer>

namespace network {
    ConnectionManager::ConnectionManager(QObject* parent)
        : QObject(parent)
        , reconnect_timer_(new QTimer(this))
    {
        reconnect_timer_->setSingleShot(true);
        connect(reconnect_timer_, &QTimer::timeout, this, &ConnectionManager::attemptReconnect);
    }

    ConnectionManager::~ConnectionManager(){
        destroyWorker();
    }

    void ConnectionManager::connectToHost(const QString& host, int port) {
        if (state_ == State::Connected ||
            state_ == State::Connecting ||
            state_ == State::Reconnecting) {
            return;
        }

        reconnect_timer_->stop();
        resetReconnectState();

        saved_host_ = host;
        saved_port_ = static_cast<quint16>(port);

        setLastError(QString{});
        setState(State::Connecting);
        createWorkerIfNeeded();

        QMetaObject::invokeMethod(
            worker_,
            "start",
            Qt::QueuedConnection,
            Q_ARG(QString, host),
            Q_ARG(quint16, static_cast<quint16>(port)));
    }

    void ConnectionManager::disconnectFromHost() {
        if (state_ == State::Disconnected || state_ == State::Disconnecting) {
            return;
        }

        reconnect_timer_->stop();
        resetReconnectState();

        setState(State::Disconnecting);
        if (worker_){
            QMetaObject::invokeMethod(worker_, "stop", Qt::QueuedConnection);
        } else {
            setState(State::Disconnected);
        }
    }

    bool ConnectionManager::isConnected() const {
        return connected_;
    }

    ConnectionManager::State ConnectionManager::state() const {
        return state_;
    }

    QString ConnectionManager::lastError() const {
        return last_error_;
    }

    void ConnectionManager::setState(State newState) {
        if (state_ == newState)
            return;
        state_ = newState;

        emit stateChanged(state_);

        bool nowConnected = (state_ == State::Connected);
        setConnected(nowConnected);

        if (state_ == State::Connected) {
            resetReconnectState();
        }
    }

    void ConnectionManager::setConnected(bool connected) {
        if (connected_ == connected)
            return;
        connected_ = connected;

        emit connectedChanged(connected_);
    }

    void ConnectionManager::setLastError(const QString& message) {
        if (last_error_ == message) 
            return;

        last_error_ = message;
        emit lastErrorChanged(last_error_);
    }

    void ConnectionManager::createWorkerIfNeeded() {
        if (worker_) return;

        workerThread_ = new QThread(this);
        worker_ = new TcpReaderWorker();

        worker_->moveToThread(workerThread_);

        connect(workerThread_, &QThread::finished,
                worker_, &QObject::deleteLater);
        connect(worker_, &TcpReaderWorker::connected, this, [this](){
            setState(State::Connected);
        });
        connect(worker_, &TcpReaderWorker::disconnected, this, [this]() {
            if (state_ == State::Disconnecting) {
                setState(State::Disconnected);
            } else if (state_ == State::Connected || state_ == State::Connecting) {
                setState(State::Disconnected);
                scheduleReconnect();
            }
        });
        connect(worker_, &TcpReaderWorker::errorOccurred, this, [this](const QString& message) {
            setLastError(message);
            setState(State::Error);
            scheduleReconnect();
        });
        connect(worker_, &TcpReaderWorker::laneSummaryParsed,
        this, &ConnectionManager::laneSummaryReceived);
        connect(worker_, &TcpReaderWorker::markingObjectsParsed,
                this, &ConnectionManager::markingObjectsReceived);
        // connect(worker_, &TcpReaderWorker::parseErrorOccurred,
        //         this, &ConnectionManager::parseErrorOccurred);

        workerThread_->start();
    }

    void ConnectionManager::destroyWorker(){
        if (!workerThread_)
            return;

        workerThread_->quit();
        workerThread_->wait();
        workerThread_->deleteLater();
        workerThread_ = nullptr;
        worker_ = nullptr;
    }

    void ConnectionManager::scheduleReconnect() {
        if (!auto_reconnect_) {
            LOG_DEBUG << "Auto-reconnect disabled, skipping reconnect";
            return;
        }

        if (saved_host_.isEmpty()) {
            LOG_WARN << "Cannot reconnect: no saved host";
            return;
        }

        if (max_reconnect_attempts_ > 0 && current_reconnect_attempt_ >= max_reconnect_attempts_) {
            LOG_ERROR << "Max reconnect attempts (" << max_reconnect_attempts_ << ") reached";
            setLastError("Max reconnect attempts reached");
            return;
        }

        current_reconnect_attempt_++;
        LOG_INFO << "Scheduling reconnect attempt " << current_reconnect_attempt_
                 << (max_reconnect_attempts_ > 0 ? " of " + std::to_string(max_reconnect_attempts_) : " (unlimited)")
                 << " in " << reconnect_interval_ << "ms";

        emit reconnectAttempt(current_reconnect_attempt_, max_reconnect_attempts_);

        setState(State::Reconnecting);
        reconnect_timer_->start(reconnect_interval_);
    }

    void ConnectionManager::attemptReconnect() {
        if (saved_host_.isEmpty()) {
            LOG_WARN << "Reconnect attempt aborted: no saved host";
            return;
        }

        if (state_ != State::Reconnecting) {
            LOG_WARN << "Reconnect attempt aborted: invalid state";
            return;
        }

        LOG_INFO << "Attempting to reconnect to " << saved_host_.toStdString() << ":" << saved_port_;

        setState(State::Connecting);
        createWorkerIfNeeded();

        QMetaObject::invokeMethod(
            worker_,
            "start",
            Qt::QueuedConnection,
            Q_ARG(QString, saved_host_),
            Q_ARG(quint16, saved_port_));
    }

    void ConnectionManager::resetReconnectState() {
        if (current_reconnect_attempt_ > 0) {
            LOG_INFO << "Resetting reconnect state (was at attempt " << current_reconnect_attempt_ << ")";
        }
        current_reconnect_attempt_ = 0;
    }

    void ConnectionManager::setAutoReconnect(bool enabled) {
        if (auto_reconnect_ == enabled)
            return;
        auto_reconnect_ = enabled;

        if (!enabled) {
            reconnect_timer_->stop();
            resetReconnectState();
        }
    }

    bool ConnectionManager::autoReconnect() const {
        return auto_reconnect_;
    }

    void ConnectionManager::setReconnectInterval(int milliseconds) {
        if (reconnect_interval_ == milliseconds)
            return;
        reconnect_interval_ = milliseconds;
    }

    int ConnectionManager::reconnectInterval() const {
        return reconnect_interval_;
    }

    void ConnectionManager::setMaxReconnectAttempts(int attempts) {
        if (max_reconnect_attempts_ == attempts)
            return;
        max_reconnect_attempts_ = attempts;
    }

    int ConnectionManager::maxReconnectAttempts() const {
        return max_reconnect_attempts_;
    }

    int ConnectionManager::currentReconnectAttempt() const {
        return current_reconnect_attempt_;
    }

}