#pragma once

#include "AbstractVideoWidget.hpp"
#include "QtMultimediaVideoProvider.hpp"

namespace video
{
    class NetworkVideoWidget : public AbstractVideoWidget
    {
        Q_OBJECT
        Q_PROPERTY(QString sourceUrl READ sourceUrl WRITE setSourceUrl NOTIFY sourceUrlChanged)
        Q_PROPERTY(bool autoStart READ autoStart WRITE setAutoStart NOTIFY autoStartChanged)
        Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

    public:
        explicit NetworkVideoWidget(QWidget* parent = nullptr);
        ~NetworkVideoWidget() override;

        void setSourceUrl(const QString& url);
        [[nodiscard]] QString sourceUrl() const;

        void setAutoStart(bool enabled);
        [[nodiscard]] bool autoStart() const;

        [[nodiscard]] bool isConnected() const;

        void connectToSource();
        void disconnectFromSource();

    signals:
        void sourceUrlChanged(const QString& url);
        void autoStartChanged(bool enabled);
        void connectedChanged(bool connected);
        void connectionFailed(const QString& error);
        void connectionEstablished();

        void frameDisplayed(quint64 timestamp_ms);

    private slots:
        void onProviderStateChangedInternal(IVideoFrameProvider::ProviderState state);

    protected:
        void paintEvent(QPaintEvent* event) override;

    private:
        QtMultimediaVideoProvider* m_videoProvider;
        QString m_sourceUrl;
        bool m_autoStart = false;
        bool m_connected = false;

        void updateConnectionState(bool connected);
    };

} // namespace video