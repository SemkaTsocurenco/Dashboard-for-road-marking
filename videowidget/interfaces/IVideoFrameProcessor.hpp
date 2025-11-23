#pragma once

#include <QSharedPointer>
#include <QString>
#include <functional>
#include "IFrameHandle.hpp"

namespace video {

    class IVideoFrameProcessor
    {
    public:
        using ProcessingCallback = std::function<void(bool success, const QString& error)>;

        virtual ~IVideoFrameProcessor() = default;

        virtual void processFrame(const FrameHandlePtr& frame) = 0;
        virtual void processFrameAsync(const FrameHandlePtr& frame, ProcessingCallback callback) = 0;
        [[nodiscard]] virtual bool isProcessing() const = 0;
        virtual void cancel() = 0;
        [[nodiscard]] virtual QString name() const = 0;
        virtual void reset() = 0;
    };

    using FrameProcessorPtr = QSharedPointer<IVideoFrameProcessor>;

} // namespace video