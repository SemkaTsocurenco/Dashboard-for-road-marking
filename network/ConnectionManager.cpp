#include "ConnectionManager.h"
#include <qnamespace.h>
#include <qobjectdefs.h>

namespace network {
    ConnectionManager::ConnectionManager(QObject* parent)
        : QObject(parent) {
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
            setState(State::Connected);});
        connect(worker_, &TcpReaderWorker::disconnected, this, [this]() {
            if (state_ == State::Disconnecting || state_ == State::Connected)
                setState(State::Disconnected);});
        connect(worker_, &TcpReaderWorker::errorOccurred,this, [this](const QString& message) {
                setLastError(message);
                setState(State::Error);
            });

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


}