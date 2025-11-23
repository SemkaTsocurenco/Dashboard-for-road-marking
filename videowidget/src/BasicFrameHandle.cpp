#include "BasicFrameHandle.hpp"

using namespace video;

BasicFrameHandle::BasicFrameHandle(const QImage& image)
    : m_image(image)
{
}

const QImage& BasicFrameHandle::image() const
{
    return m_image;
}

QImage& BasicFrameHandle::writableImage()
{
    return m_image;
}

bool BasicFrameHandle::isValid() const
{
    return !m_image.isNull();
}




