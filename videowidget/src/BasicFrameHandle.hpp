#pragma once

#include <QImage>
#include <qimage.h>
#include "IFrameHandle.hpp"

namespace video {
    class BasicFrameHandle : public IFrameHandle 
    {
    public:
        BasicFrameHandle() = default;
        explicit BasicFrameHandle(const QImage& image);
        ~BasicFrameHandle() override = default;
        
        const QImage& image() const override;
        QImage& writableImage() override;
        [[nodiscard]] bool isValid() const override;
    private:
        QImage m_image;
    };
} // namespace video