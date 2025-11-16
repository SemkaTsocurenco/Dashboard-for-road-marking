#pragma once 

#include <QObject>
#include <QTcpSocket>
#include "proto_parser.h"

namespace network {
    class TcpReaderWorker : public QObject 
    {
        Q_OBJECT
    public:
        explicit TcpReaderWorker(QObject* parent = nullptr);
        ~TcpReaderWorker() override;
    
    public slots: 
        void start(const QString& host, quint16 port);
        void stop();

    signals: 
        void connected();
        void disconnected();
        void errorOccurred(const QString& message);  
        void laneSummaryParsed (const laneproto::LaneSummary& msg);
        void markingObjectsParsed(const laneproto::MarkingObjects& msg);
        void parseErrorOccurred(const laneproto::ParseError& error);
    
    private slots: 
        void onSocketConnected();
        void onSocketDisconnected();
        void onSocketError(QAbstractSocket::SocketError socketError);
        void onReadyRead();

    private:
        QString host_;
        quint16 port_{0};
        QTcpSocket* socket_{nullptr};

        // для переброса из парсера в воркер
        class MessageHandler : public laneproto::IMessageHandler{
        public:
            explicit MessageHandler (TcpReaderWorker& owner) noexcept 
                : owner_(owner){};
            
            void onLaneSummary (const laneproto::LaneSummary& msg) override;
            void onMarkingObjects(const laneproto::MarkingObjects& msg) override;
            void onParseError(const laneproto::ParseError& error) override;
        private:
            TcpReaderWorker& owner_;
        };

        MessageHandler handler_{*this};
        laneproto::ProtoParser parser_{handler_};
    };
}