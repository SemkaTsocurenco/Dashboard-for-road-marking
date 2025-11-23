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

int BasicFrameHandle::width() const
{
    return m_image.width();
}

int BasicFrameHandle::height() const
{
    return m_image.height();
}

int64_t BasicFrameHandle::timestamp() const
{
    return m_timestamp;
}

void BasicFrameHandle::setTimestamp(int64_t timestamp)
{
    m_timestamp = timestamp;
}

IFrameHandle* BasicFrameHandle::clone() const
{
    auto* copy = new BasicFrameHandle(m_image);
    copy->setTimestamp(m_timestamp);
    return copy;
}

