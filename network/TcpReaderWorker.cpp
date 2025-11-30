#include "TcpReaderWorker.h"
#include "LoggerMacros.hpp"
#include <QByteArray>
#include <QHostAddress>
#include <cstdint>
#include <cstddef>

namespace network {
    TcpReaderWorker::TcpReaderWorker(QObject* parent)
        : QObject(parent)
        , server_(new QTcpServer(this))
        , socket_(nullptr)
    {
        connect(server_, &QTcpServer::newConnection,
                this, &TcpReaderWorker::onNewConnection);
        connect(server_, &QTcpServer::acceptError,
                this, &TcpReaderWorker::onServerError);
    }

    TcpReaderWorker::~TcpReaderWorker()
    {
        stop();
    }

    void TcpReaderWorker::start(const QString& host, quint16 port)
    {
        host_ = host;
        port_ = port;

        // Reset parser state before new connection
        parser_.reset();
        LOG_DEBUG << "Parser state reset for new connection";

        // Tear down existing client connection if any
        if (socket_) {
            socket_->disconnect(this);
            socket_->disconnectFromHost();
            socket_->deleteLater();
            socket_ = nullptr;
        }

        if (server_->isListening()) {
            server_->close();
        }

        QHostAddress bind_addr;
        if (!bind_addr.setAddress(host_)) {
            LOG_WARN << "Invalid bind address '" << host_.toStdString()
                     << "', falling back to 0.0.0.0";
            bind_addr = QHostAddress::Any;
        }

        if (!server_->listen(bind_addr, port_)) {
            LOG_ERROR << "Failed to listen on " << host_.toStdString()
                      << ":" << port_ << " - " << server_->errorString().toStdString();
            emit errorOccurred(server_->errorString());
            return;
        }

        LOG_INFO << "Listening for incoming TCP on "
                 << server_->serverAddress().toString().toStdString()
                 << ":" << server_->serverPort();
    }

    void TcpReaderWorker::stop()
    {
        if (socket_) {
            LOG_INFO << "Closing client connection from "
                     << socket_->peerAddress().toString().toStdString()
                     << ":" << socket_->peerPort();
            socket_->disconnectFromHost();
            socket_->deleteLater();
            socket_ = nullptr;
        }
        if (server_->isListening()) {
            LOG_INFO << "Stopping listener on " << host_.toStdString() << ":" << port_;
            server_->close();
        }
    }

    void TcpReaderWorker::onSocketConnected()
    {
        if (!socket_)
            return;

        LOG_INFO << "Accepted connection from "
                 << socket_->peerAddress().toString().toStdString()
                 << ":" << socket_->peerPort();
        emit connected();
    }

    void TcpReaderWorker::onSocketDisconnected()
    {
        QString peer = socket_ ? socket_->peerAddress().toString() : host_;
        quint16 peer_port = socket_ ? socket_->peerPort() : port_;
        LOG_WARN << "Disconnected from " << peer.toStdString() << ":" << peer_port;

        if (socket_) {
            socket_->deleteLater();
            socket_ = nullptr;
        }
        emit disconnected();
    }

    void TcpReaderWorker::onSocketError(QAbstractSocket::SocketError socketError)
    {
        Q_UNUSED(socketError);
        QString err = socket_ ? socket_->errorString() : QStringLiteral("Unknown socket error");
        LOG_ERROR << "Socket error: " << err.toStdString();
        emit errorOccurred(err);
    }

    void TcpReaderWorker::onServerError(QAbstractSocket::SocketError socketError)
    {
        Q_UNUSED(socketError);
        QString err = server_ ? server_->errorString() : QStringLiteral("Unknown server error");
        LOG_ERROR << "Server error: " << err.toStdString();
        emit errorOccurred(err);
    }

    void TcpReaderWorker::onNewConnection()
    {
        while (server_->hasPendingConnections()) {
            QTcpSocket* client = server_->nextPendingConnection();
            if (!client)
                continue;

            if (socket_) {
                LOG_WARN << "Replacing existing client connection from "
                         << socket_->peerAddress().toString().toStdString()
                         << ":" << socket_->peerPort();
                socket_->disconnect(this);
                socket_->disconnectFromHost();
                socket_->deleteLater();
                socket_ = nullptr;
            }

            socket_ = client;

            connect(socket_, &QTcpSocket::disconnected,
                    this, &TcpReaderWorker::onSocketDisconnected);
            connect(socket_, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
                    this, &TcpReaderWorker::onSocketError);
            connect(socket_, &QTcpSocket::readyRead,
                    this, &TcpReaderWorker::onReadyRead);

            parser_.reset();
            LOG_DEBUG << "Parser state reset on new client connection";
            onSocketConnected();
        }
    }

    void TcpReaderWorker::onReadyRead()
    {
        if (!socket_)
            return;

        QByteArray data = socket_->readAll();
        if (data.isEmpty())
            return;

        LOG_TRACE << "Received " << data.size() << " bytes from socket";

        const auto* raw = reinterpret_cast<const std::uint8_t*>(data.constData());
        const std::size_t size = static_cast<std::size_t>(data.size());

        parser_.feed(raw, size);
    }

    void TcpReaderWorker::MessageHandler::onLaneSummary(const laneproto::LaneSummary& msg){
        LOG_DEBUG << "LaneSummary received: seq=" << static_cast<int>(msg.seq)
                  << ", timestamp=" << msg.timestamp_ms
                  << ", left_offset=" << msg.left_offset_m
                  << ", right_offset=" << msg.right_offset_m;
        owner_.laneSummaryParsed(msg);
    }

    void TcpReaderWorker::MessageHandler::onMarkingObjects(const laneproto::MarkingObjects& msg){
        LOG_DEBUG << "MarkingObjects received: seq=" << static_cast<int>(msg.seq)
                  << ", timestamp=" << msg.timestamp_ms
                  << ", objects=" << msg.objects.size();
        owner_.markingObjectsParsed(msg);
    }

    void TcpReaderWorker::MessageHandler::onLaneDetails(const laneproto::LaneDetails& msg){
        LOG_DEBUG << "LaneDetails received: seq=" << static_cast<int>(msg.seq)
                  << ", timestamp=" << msg.timestamp_ms
                  << ", left_pts=" << static_cast<int>(msg.left.points_count)
                  << ", right_pts=" << static_cast<int>(msg.right.points_count);
        owner_.laneDetailsParsed(msg);
    }

    void TcpReaderWorker::MessageHandler::onMarkingObjectsEx(const laneproto::MarkingObjects& msg){
        LOG_DEBUG << "MarkingObjectsEx received: seq=" << static_cast<int>(msg.seq)
                  << ", timestamp=" << msg.timestamp_ms
                  << ", objects=" << msg.objects.size();
        owner_.markingObjectsExParsed(msg);
    }

    void TcpReaderWorker::MessageHandler::onParseError(const laneproto::ParseError& error){
        std::string error_code_str;
        switch (error.code) {
            case laneproto::ParseErrorCode::Unknown:
                error_code_str = "Unknown";
                break;
            case laneproto::ParseErrorCode::BadVersion:
                error_code_str = "BadVersion";
                break;
            case laneproto::ParseErrorCode::PayloadTooLong:
                error_code_str = "PayloadTooLong";
                break;
            case laneproto::ParseErrorCode::HeaderTruncated:
                error_code_str = "HeaderTruncated";
                break;
            case laneproto::ParseErrorCode::PayloadTruncated:
                error_code_str = "PayloadTruncated";
                break;
            case laneproto::ParseErrorCode::CrcMismatch:
                error_code_str = "CrcMismatch";
                break;
            case laneproto::ParseErrorCode::UnknownMsgType:
                error_code_str = "UnknownMsgType";
                break;
            case laneproto::ParseErrorCode::LaneSummaryFormat:
                error_code_str = "LaneSummaryFormat";
                break;
            case laneproto::ParseErrorCode::MarkingFormat:
                error_code_str = "MarkingFormat";
                break;
            case laneproto::ParseErrorCode::LaneDetailsFormat:
                error_code_str = "LaneDetailsFormat";
                break;
            case laneproto::ParseErrorCode::MarkingExFormat:
                error_code_str = "MarkingExFormat";
                break;
        }

        LOG_ERROR << "Parse error [" << error_code_str << "]: " << error.message;
        owner_.parseErrorOccurred(error);
    }
}
