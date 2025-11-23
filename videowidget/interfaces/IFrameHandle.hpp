#pragma once

#include <QImage>
#include <QSharedPointer>
#include <cstdint>

namespace video {

    class IFrameHandle
    {
    public:
        virtual ~IFrameHandle() = default;

        virtual const QImage& image() const = 0;
        virtual QImage& writableImage() = 0;
        [[nodiscard]] virtual bool isValid() const = 0;
        [[nodiscard]] virtual int width() const = 0;
        [[nodiscard]] virtual int height() const = 0;
        [[nodiscard]] virtual int64_t timestamp() const = 0;
        virtual void setTimestamp(int64_t timestamp) = 0;
        virtual IFrameHandle* clone() const = 0;
    };

    using FrameHandlePtr = QSharedPointer<IFrameHandle>;

}// namespace video