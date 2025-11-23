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
            Error
        };
        Q_ENUM(ProviderState)

        explicit IVideoFrameProvider(QObject* parent = nullptr) 
            : QObject(parent)
        {}
        ~IVideoFrameProvider() override = default;

        virtual void start() = 0;
        virtual void stop() = 0;
        [[nodiscard]] virtual bool isRunning() const = 0;

    signals: 
        void frameReady(const FrameHandlePtr& frame);
        void errorOccurred(const QString& message);
        void stateChanged(ProviderState state);

    };

    using FrameProviderPtr = QSharedPointer<IVideoFrameProvider>;

} // namespace video
