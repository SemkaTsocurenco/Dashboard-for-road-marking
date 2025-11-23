#pragma once

#include <QMediaPlayer>
#include <QVideoSink>
#include <QElapsedTimer>
#include <QVideoFrame>

#include "IVideoFrameProvider.hpp"
#include "BasicFrameHandle.hpp"

namespace video {
    class QtMultimediaVideoProvider : public IVideoFrameProvider
    {
        Q_OBJECT
    public:
        explicit QtMultimediaVideoProvider(QObject* parent = nullptr);
        ~QtMultimediaVideoProvider() override;

        void start() override;
        void stop() override;
        void pause() override;
        void resume() override;
        [[nodiscard]] bool isRunning() const override;
        [[nodiscard]] ProviderState state() const override;
        [[nodiscard]] QString source() const override;
        void setSource(const QString& source) override;
        [[nodiscard]] double frameRate() const override;

    private slots:
        void onVideoFrameChanged(const QVideoFrame& frame);
        void onMediaError(QMediaPlayer::Error error, const QString& errorString);
        void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
        void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);

    private:
        QMediaPlayer m_player;
        QVideoSink  m_videoSink;

        QString m_source;
        ProviderState m_state = ProviderState::Stopped;
        bool m_running = false;

        QElapsedTimer m_fpsTimer;
        int m_framesInSecond = 0;
        double m_currentFps = 0.0;

        void updateState(ProviderState newState);
        void updateFps();
    };
}