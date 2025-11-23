#include "QtMultimediaVideoProvider.hpp"
#include "LoggerMacros.hpp"

#include <QUrl>
#include <QDateTime>

using namespace video;

QtMultimediaVideoProvider::QtMultimediaVideoProvider (QObject* parent)
    : IVideoFrameProvider(parent)
{
    LOG_TRACE << "QtMultimediaVideoProvider created";

    m_player.setVideoOutput(&m_videoSink);

    connect(&m_videoSink, &QVideoSink::videoFrameChanged,
            this, &QtMultimediaVideoProvider::onVideoFrameChanged);
    connect(&m_player, &QMediaPlayer::errorOccurred,
            this, &QtMultimediaVideoProvider::onMediaError);
    connect(&m_player, &QMediaPlayer::mediaStatusChanged,
            this, &QtMultimediaVideoProvider::onMediaStatusChanged);
    connect(&m_player, &QMediaPlayer::playbackStateChanged,
            this, &QtMultimediaVideoProvider::onPlaybackStateChanged);
}

QtMultimediaVideoProvider::~QtMultimediaVideoProvider()
{
    LOG_TRACE << "QtMultimediaVideoProvider destroyed";
}

void QtMultimediaVideoProvider::setSource(const QString& source)
{
    if (m_source == source)
        return;

    m_source = source;
    LOG_INFO << "Source set to " << m_source.toStdString();
    emit sourceChanged(m_source);
}

QString QtMultimediaVideoProvider::source() const 
{
    return m_source;
}

void QtMultimediaVideoProvider::start()
{
    if (m_source.isEmpty()){
        LOG_ERROR << "Source is not set";
        emit errorOccurred("Empty source");
        updateState(ProviderState::Error);
        return;
    }

    m_player.setSource(QUrl::fromUserInput(m_source));
    m_fpsTimer.restart();
    m_framesInSecond = 0;

    m_running = true;
    updateState(ProviderState::Starting);

    LOG_INFO << "Starting playback for source" << m_source.toStdString();
    m_player.play();
}


void QtMultimediaVideoProvider::stop()
{
    if (!m_running)
        return;

    LOG_INFO << "Stopping playback";
    m_player.stop();
    m_running = false;
    updateState(ProviderState::Stopped);

    m_currentFps = 0.0;
    m_framesInSecond = 0;
    m_fpsTimer.invalidate();
}

void QtMultimediaVideoProvider::pause()
{
    if (!m_running) {
        LOG_WARN << "Cannot pause: not running";
        return;
    }

    LOG_INFO << "Pausing playback";
    m_player.pause();
    updateState(ProviderState::Paused);
}

void QtMultimediaVideoProvider::resume()
{
    if (!m_running) {
        LOG_WARN << "Cannot resume: not running";
        return;
    }

    if (m_state != ProviderState::Paused) {
        LOG_WARN << "Cannot resume: not paused";
        return;
    }

    LOG_INFO << "Resuming playback";
    m_player.play();
    updateState(ProviderState::Running);
}

bool QtMultimediaVideoProvider::isRunning() const
{
    return m_running;
}

IVideoFrameProvider::ProviderState QtMultimediaVideoProvider::state() const
{
    return m_state;
}


double QtMultimediaVideoProvider::frameRate() const 
{
    return m_currentFps;
}

void QtMultimediaVideoProvider::updateState(ProviderState newState)
{
    if (m_state == newState)
        return;

    m_state = newState;
    LOG_INFO << "Provider state changed to" << static_cast<int>(m_state);
    emit stateChanged(m_state);
}


void QtMultimediaVideoProvider::onVideoFrameChanged(const QVideoFrame& frame)
{
    LOG_TRACE << "Video frame changed";
    QImage img = frame.toImage();

    if (img.isNull()){
        LOG_WARN << "Image is null";
        return;
    }

    FrameHandlePtr handle(new BasicFrameHandle(img));
    const auto ts = QDateTime::currentMSecsSinceEpoch();
    handle->setTimestamp(ts);

    m_framesInSecond++;
    updateFps();

    emit frameReady(handle);
    if (m_state == ProviderState::Starting)
        updateState(ProviderState::Running);
}

void QtMultimediaVideoProvider::onMediaError(QMediaPlayer::Error error, const QString& errorString)
{
    LOG_ERROR << "Media error:" << static_cast<int>(error) << errorString.toStdString();
    emit errorOccurred(errorString);
    updateState(ProviderState::Error);
    m_running = false;
    m_player.stop();
}

void QtMultimediaVideoProvider::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    LOG_DEBUG << "Media status changed to" << static_cast<int>(status);

    switch (status) {
        case QMediaPlayer::EndOfMedia:
            LOG_INFO << "End of media reached";
            m_running = false;
            updateState(ProviderState::Stopped);
            break;
        case QMediaPlayer::InvalidMedia:
            LOG_ERROR << "Invalid media";
            emit errorOccurred("Invalid media");
            updateState(ProviderState::Error);
            m_running = false;
            break;
        case QMediaPlayer::LoadedMedia:
            LOG_INFO << "Media loaded";
            break;
        default:
            break;
    }
}

void QtMultimediaVideoProvider::onPlaybackStateChanged(QMediaPlayer::PlaybackState state)
{
    LOG_DEBUG << "Playback state changed to" << static_cast<int>(state);

    switch (state) {
        case QMediaPlayer::PlayingState:
            if (m_state == ProviderState::Paused)
                updateState(ProviderState::Running);
            break;
        case QMediaPlayer::PausedState:
            if (m_running)
                updateState(ProviderState::Paused);
            break;
        case QMediaPlayer::StoppedState:
            if (m_running) {
                m_running = false;
                updateState(ProviderState::Stopped);
            }
            break;
    }
}

void QtMultimediaVideoProvider::updateFps()
{
    const qint64 elapsed = m_fpsTimer.elapsed();

    if (elapsed >= 1000) {
        m_currentFps = (m_framesInSecond * 1000.0) / elapsed;
        LOG_DEBUG << "Current FPS:" << m_currentFps;
        m_framesInSecond = 0;
        m_fpsTimer.restart();
    }
}