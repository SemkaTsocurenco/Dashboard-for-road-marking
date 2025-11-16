#include "TcpReaderWorker.h"
#include "LoggerMacros.hpp"
#include <QByteArray>
#include <cstdint>
#include <cstddef>

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
        LOG_INFO << "Connecting to " << host.toStdString() << ":" << port;
        socket_->connectToHost(host_, port_);
    }

    void TcpReaderWorker::stop()
    {
        if (socket_->isOpen()) {
            LOG_INFO << "Disconnecting from " << host_.toStdString() << ":" << port_;
            socket_->disconnectFromHost();
        }
    }

    void TcpReaderWorker::onSocketConnected()
    {
        LOG_INFO << "Successfully connected to " << host_.toStdString() << ":" << port_;
        emit connected();
    }

    void TcpReaderWorker::onSocketDisconnected()
    {
        LOG_WARN << "Disconnected from " << host_.toStdString() << ":" << port_;
        emit disconnected();
    }

    void TcpReaderWorker::onSocketError(QAbstractSocket::SocketError socketError)
    {
        Q_UNUSED(socketError);
        LOG_ERROR << "Socket error: " << socket_->errorString().toStdString();
        emit errorOccurred(socket_->errorString());
    }

    void TcpReaderWorker::onReadyRead()
    {
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
        }

        LOG_ERROR << "Parse error [" << error_code_str << "]: " << error.message;
        owner_.parseErrorOccurred(error);
    }
}