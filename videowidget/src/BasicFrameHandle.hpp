#pragma once

#include <QImage>
#include <cstdint>
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
        [[nodiscard]] int width() const override;
        [[nodiscard]] int height() const override;
        [[nodiscard]] int64_t timestamp() const override;
        void setTimestamp(int64_t timestamp) override;
        IFrameHandle* clone() const override;
    private:
        QImage m_image;
        int64_t m_timestamp = 0;
    };
} // namespace video