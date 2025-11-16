#pragma once 

#include <QObject>
#include <QTcpSocket>

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
    

    private slots: 
        void onSocketConnected();
        void onSocketDisconnected();
        void onSocketError(QAbstractSocket::SocketError socketError);
        void onReadyRead();

    private:
        QString host_;
        quint16 port_{0};
        QTcpSocket* socket_{nullptr};
    };
}