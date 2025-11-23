#pragma once

#include <QWidget>
#include <QVector>
#include <QImage>

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
        [[nodiscard]] bool isRunning() const;

        // процессоры
        void addFrameProcessor(const FrameProcessorPtr& processor);
        void clearFrameProcessors();

        // доступ к последнему кадру 
        [[nodiscard]] FrameHandlePtr lastFrameHandle() const;
        [[nodiscard]] QImage lastFrameImage() const;

        // настройки отображения
        void setAspectRatioMode(Qt::AspectRatioMode mode);
        [[nodiscard]] Qt::AspectRatioMode aspectRatioMode() const;

        void setMaintainAspectRatio(bool enabled);
        [[nodiscard]] bool maintainAspectRatio() const;

    private:
        IVideoFrameProvider* m_provider = nullptr;
        FrameHandlePtr m_lastFrame;
        QVector<FrameProcessorPtr> m_processors;
        Qt::AspectRatioMode m_aspectRatioMode = Qt::KeepAspectRatio;
        bool m_maintainAspectRatio = true;

    signals:
        void frameUpdated(const FrameHandlePtr& frame);
        void errorOccurred(const QString& message);
        void providerStateChanged(IVideoFrameProvider::ProviderState state);

    private slots:
        void onFrameReady(const FrameHandlePtr& frame);
        void onProviderError(const QString& message);
        void onProviderStateChanged(IVideoFrameProvider::ProviderState state);

    protected:
        void paintEvent(QPaintEvent* event) override;

    };

} // namespace video