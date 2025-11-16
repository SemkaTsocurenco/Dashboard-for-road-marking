#include "TcpReaderWorker.h"
#include <QByteArray>

namespace network {
    TcpReaderWorker::TcpReaderWorker(QObject* parent)
        : QObject(parent)
        , socket_(new QTcpSocket(this))
    {
        connect(socket_, &QTcpSocket::connected, this, &TcpReaderWorker::onSocketConnected);
        connect(socket_, &QTcpSocket::disconnected, this, &TcpReaderWorker::onSocketDisconnected);
        connect(socket_, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
                this, &TcpReaderWorker::onSocketError);
        connect(socket_, &QTcpSocket::readyRead, this, &TcpReaderWorker::onReadyRead);
    }

    TcpReaderWorker::~TcpReaderWorker()
    {
        stop();
    }

    void TcpReaderWorker::start(const QString& host, quint16 port)
    {
        host_ = host;
        port_ = port;
        socket_->connectToHost(host_, port_);
    }

    void TcpReaderWorker::stop()
    {
        if (socket_->isOpen()) {
            socket_->disconnectFromHost();
        }
    }

    void TcpReaderWorker::onSocketConnected()
    {
        emit connected();
    }

    void TcpReaderWorker::onSocketDisconnected()
    {
        emit disconnected();
    }

    void TcpReaderWorker::onSocketError(QAbstractSocket::SocketError socketError)
    {
        Q_UNUSED(socketError);
        emit errorOccurred(socket_->errorString());
    }

    void TcpReaderWorker::onReadyRead()
    {
        // Handle incoming data here
        QByteArray data = socket_->readAll();
        // Process data...
    }
}