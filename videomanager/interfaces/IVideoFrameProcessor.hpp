#pragma once

#include "IFrameHandle.hpp"

namespace video {
    class IVideoFrameProcessor
    {
    public:
        virtual ~IVideoFrameProcessor() = default;

        virtual void processFrame(const FrameHandlePtr& frame) = 0;
    };

} // namespace video