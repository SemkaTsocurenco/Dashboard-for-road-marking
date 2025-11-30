#pragma once

#include <QElapsedTimer>
#include <QTimer>
#include <atomic>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/video/video.h>

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
        void pollBusMessages();

    private:
        QString buildPipelineDescription(QString& error) const;
        bool setupPipeline(QString& error);
        void cleanupPipeline();
        void handlePipelineError(const QString& message);
        void resetFps();
        void updateState(ProviderState newState);

        static GstFlowReturn onNewSample(GstAppSink* sink, gpointer user_data);
        void handleSample(GstSample* sample);
        void handleBusMessage(GstMessage* message);
        void updateFps();

        GstElement* m_pipeline = nullptr;
        GstAppSink* m_appSink = nullptr;
        GstBus* m_bus = nullptr;

        QString m_source;
        ProviderState m_state = ProviderState::Stopped;
        bool m_running = false;

        QElapsedTimer m_fpsTimer;
        int m_framesInSecond = 0;
        double m_currentFps = 0.0;

        QTimer m_busTimer;
    };
}
