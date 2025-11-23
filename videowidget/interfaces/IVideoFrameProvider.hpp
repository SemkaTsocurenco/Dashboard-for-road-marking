#pragma once

#include <QObject>
#include <QString>
#include <QSharedPointer>
#include "IFrameHandle.hpp"

namespace video {

    class IVideoFrameProvider : public QObject
    {
        Q_OBJECT

    public:
        enum class ProviderState
        {
            Stopped,
            Starting,
            Running,
            Paused,
            Error
        };
        Q_ENUM(ProviderState)

        explicit IVideoFrameProvider(QObject* parent = nullptr) {};
        ~IVideoFrameProvider() override = default;

        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void pause() = 0;
        virtual void resume() = 0;
        [[nodiscard]] virtual bool isRunning() const = 0;
        [[nodiscard]] virtual ProviderState state() const = 0;
        [[nodiscard]] virtual QString source() const = 0;
        virtual void setSource(const QString& source) = 0;
        [[nodiscard]] virtual double frameRate() const = 0;

    signals:
        void frameReady(const FrameHandlePtr& frame);
        void errorOccurred(const QString& message);
        void stateChanged(ProviderState state);
        void sourceChanged(const QString& source);
    };

    using FrameProviderPtr = QSharedPointer<IVideoFrameProvider>;

} // namespace video
