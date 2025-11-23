#pragma once
#include <QImage>
#include <QSharedPointer>


namespace video {
    
    class IFrameHandle 
    {
    public: 
        virtual ~IFrameHandle() = default;

        virtual const QImage& image() const = 0;
        virtual QImage& writableImage() = 0;
        [[nodiscard]] virtual bool isValid() const = 0;

    };

    using FrameHandlePtr = QSharedPointer<IFrameHandle>;

}// namespace video