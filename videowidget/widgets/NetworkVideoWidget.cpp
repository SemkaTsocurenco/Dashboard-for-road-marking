#include "NetworkVideoWidget.hpp"
#include "LoggerMacros.hpp"

using namespace video;

NetworkVideoWidget::NetworkVideoWidget(QWidget* parent)
    : AbstractVideoWidget(parent)
    , m_videoProvider(new QtMultimediaVideoProvider(this))
{
    LOG_TRACE << "NetworkVideoWidget created";

    setFrameProvider(m_videoProvider);

    connect(m_videoProvider, &IVideoFrameProvider::stateChanged,
            this, &NetworkVideoWidget::onProviderStateChangedInternal);
    connect(m_videoProvider, &IVideoFrameProvider::errorOccurred,
            this, &NetworkVideoWidget::connectionFailed);
}

NetworkVideoWidget::~NetworkVideoWidget()
{
    LOG_TRACE << "NetworkVideoWidget destroyed";
    disconnectFromSource();
}

void NetworkVideoWidget::setSourceUrl(const QString& url)
{
    if (m_sourceUrl == url)
        return;

    const bool wasConnected = m_connected;
    if (wasConnected) {
        disconnectFromSource();
    }

    m_sourceUrl = url;
    m_videoProvider->setSource(url);

    LOG_INFO << "Source URL set to" << url.toStdString();
    emit sourceUrlChanged(url);

    // Если были подключены — переподключаемся.
    if (wasConnected && m_autoStart) {
        connectToSource();
    }
    // Если ещё не были подключены, но autoStart включён и URL не пустой — стартуем.
    else if (!wasConnected && m_autoStart && !m_sourceUrl.isEmpty()) {
        connectToSource();
    }
}

QString NetworkVideoWidget::sourceUrl() const
{
    return m_sourceUrl;
}

void NetworkVideoWidget::setAutoStart(bool enabled)
{
    if (m_autoStart == enabled)
        return;

    m_autoStart = enabled;
    LOG_DEBUG << "Auto start changed to" << (enabled ? "true" : "false");
    emit autoStartChanged(enabled);
}

bool NetworkVideoWidget::autoStart() const
{
    return m_autoStart;
}

bool NetworkVideoWidget::isConnected() const
{
    return m_connected;
}

void NetworkVideoWidget::connectToSource()
{
    if (m_sourceUrl.isEmpty()) {
        LOG_WARN << "Cannot connect: source URL is empty";
        emit connectionFailed("Source URL is empty");
        return;
    }

    if (m_connected) {
        LOG_WARN << "Already connected";
        return;
    }

    LOG_INFO << "Connecting to source" << m_sourceUrl.toStdString();
    start();
}

void NetworkVideoWidget::disconnectFromSource()
{
    if (!m_connected) {
        LOG_DEBUG << "Not connected, nothing to disconnect";
        return;
    }

    LOG_INFO << "Disconnecting from source";
    stop();
    updateConnectionState(false);
}

void NetworkVideoWidget::onProviderStateChangedInternal(IVideoFrameProvider::ProviderState state)
{
    LOG_DEBUG << "Provider state changed to" << static_cast<int>(state);

    switch (state) {
        case IVideoFrameProvider::ProviderState::Running:
            updateConnectionState(true);
            emit connectionEstablished();
            break;
        case IVideoFrameProvider::ProviderState::Stopped:
        case IVideoFrameProvider::ProviderState::Error:
            updateConnectionState(false);
            break;
        default:
            break;
    }
}

void NetworkVideoWidget::updateConnectionState(bool connected)
{
    if (m_connected == connected)
        return;

    m_connected = connected;
    LOG_INFO << "Connection state changed to" << (connected ? "connected" : "disconnected");
    emit connectedChanged(connected);
}

void NetworkVideoWidget::paintEvent(QPaintEvent* event)
{
    AbstractVideoWidget::paintEvent(event);

    auto frame = lastFrameHandle();
    if (frame && frame->isValid()) {
        quint64 timestamp_ms = static_cast<quint64>(frame->timestamp());
        emit frameDisplayed(timestamp_ms);
    }
}
