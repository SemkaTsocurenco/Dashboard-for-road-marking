#pragma once

#include <QWidget>
#include <QVector>
#include <QImage>
#include <QElapsedTimer>
#include <QColor>

#include "IVideoFrameProcessor.hpp"
#include "IVideoFrameProvider.hpp"


namespace video {
    class AbstractVideoWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit AbstractVideoWidget(QWidget* parent = nullptr);
        ~AbstractVideoWidget() override;

        // провайдер
        void setFrameProvider(IVideoFrameProvider* provider);
        IVideoFrameProvider* frameProvider() const;

        // управление потоком
        void start();
        void stop();
        void pause();
        void resume();
        [[nodiscard]] bool isRunning() const;
        [[nodiscard]] bool isPaused() const;

        // процессоры
        void addFrameProcessor(const FrameProcessorPtr& processor);
        void removeFrameProcessor(const FrameProcessorPtr& processor);
        void clearFrameProcessors();
        [[nodiscard]] int processorCount() const;

        // доступ к последнему кадру
        [[nodiscard]] FrameHandlePtr lastFrameHandle() const;
        [[nodiscard]] QImage lastFrameImage() const;

        // настройки отображения
        void setAspectRatioMode(Qt::AspectRatioMode mode);
        [[nodiscard]] Qt::AspectRatioMode aspectRatioMode() const;

        void setMaintainAspectRatio(bool enabled);
        [[nodiscard]] bool maintainAspectRatio() const;

        void setBackgroundColor(const QColor& color);
        [[nodiscard]] QColor backgroundColor() const;

        // FPS и статистика
        void setShowFps(bool show);
        [[nodiscard]] bool showFps() const;
        [[nodiscard]] double currentFps() const;
        [[nodiscard]] int64_t framesProcessed() const;
        void resetStatistics();

        // снимки
        [[nodiscard]] QImage captureFrame() const;
        bool saveFrame(const QString& filePath) const;

    private:
        IVideoFrameProvider* m_provider = nullptr;
        FrameHandlePtr m_lastFrame;
        QVector<FrameProcessorPtr> m_processors;
        Qt::AspectRatioMode m_aspectRatioMode = Qt::KeepAspectRatio;
        bool m_maintainAspectRatio = true;
        QColor m_backgroundColor = Qt::black;

        bool m_showFps = false;
        QElapsedTimer m_fpsTimer;
        int m_frameCounter = 0;
        double m_currentFps = 0.0;
        int64_t m_totalFrames = 0;

        void updateFpsCounter();

    signals:
        void frameUpdated(const FrameHandlePtr& frame);
        void errorOccurred(const QString& message);
        void providerStateChanged(IVideoFrameProvider::ProviderState state);
        void fpsChanged(double fps);

    private slots:
        void onFrameReady(const FrameHandlePtr& frame);
        void onProviderError(const QString& message);
        void onProviderStateChanged(IVideoFrameProvider::ProviderState state);

    protected:
        void paintEvent(QPaintEvent* event) override;
        virtual void drawOverlay(QPainter& painter);

    };

} // namespace video