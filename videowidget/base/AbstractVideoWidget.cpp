#include "AbstractVideoWidget.hpp"
#include "IVideoFrameProvider.hpp"
#include "LoggerMacros.hpp"

#include <QPainter>
#include <QPaintEvent>
#include <algorithm>

using namespace video;

AbstractVideoWidget::AbstractVideoWidget(QWidget* parent) 
    : QWidget(parent)
{
    LOG_TRACE << "Abstract video widget created";
}

AbstractVideoWidget::~AbstractVideoWidget()
{
    LOG_TRACE << "Abstract video widget deleted";
}

void AbstractVideoWidget::setFrameProvider(IVideoFrameProvider* provider){
    if (m_provider == provider) {
        LOG_DEBUG << "setFrameProvider called with same provider";
        return;
    }

    if (m_provider){
        disconnect(m_provider, nullptr, this, nullptr);
    }

    m_provider = provider;

    if (m_provider){
        connect(m_provider, &IVideoFrameProvider::frameReady,
                this, &AbstractVideoWidget::onFrameReady);
        connect(m_provider, &IVideoFrameProvider::errorOccurred,
                this, &AbstractVideoWidget::onProviderError);
        connect(m_provider, &IVideoFrameProvider::stateChanged,
                this, &AbstractVideoWidget::onProviderStateChanged);
        LOG_INFO << "Frame provider set";
    } else 
        LOG_INFO << "Frame provider cleared";
}

IVideoFrameProvider* AbstractVideoWidget::frameProvider() const
{
    return m_provider;
}


void AbstractVideoWidget::start()
{
    if (!m_provider) {
        LOG_WARN << "Cannot start: provider is null";
        return;
    }
    LOG_INFO << "Starting provider";
    m_provider->start();
}

void AbstractVideoWidget::stop()
{
    if (!m_provider) {
        LOG_WARN << "Cannot stop: provider is null";
        return;
    }
    LOG_INFO << "Stopping provider";
    m_provider->stop();
}

bool AbstractVideoWidget::isRunning() const
{
    return m_provider ? m_provider->isRunning() : false;
}

void AbstractVideoWidget::addFrameProcessor(const FrameProcessorPtr& processor)
{
    if (!processor) {
        LOG_WARN << "Tried to add null frame processor";
        return;
    }
    m_processors.push_back(processor);
    LOG_DEBUG << "Added frame processor, count =" << m_processors.size();
}

void AbstractVideoWidget::clearFrameProcessors()
{
    m_processors.clear();
    LOG_INFO << "Frame processors cleared";
}

FrameHandlePtr AbstractVideoWidget::lastFrameHandle() const
{
    return m_lastFrame;
}

QImage AbstractVideoWidget::lastFrameImage() const
{
    if (!m_lastFrame || !m_lastFrame->isValid())
        return {};

    return m_lastFrame->image();
}

void AbstractVideoWidget::setAspectRatioMode(Qt::AspectRatioMode mode)
{
    if (m_aspectRatioMode == mode)
        return;

    m_aspectRatioMode = mode;
    LOG_DEBUG << "Aspect ratio mode changed to" << static_cast<int>(mode);
    update(); 
}

Qt::AspectRatioMode AbstractVideoWidget::aspectRatioMode() const
{
    return m_aspectRatioMode;
}

void AbstractVideoWidget::setMaintainAspectRatio(bool enabled)
{
    if (m_maintainAspectRatio == enabled)
        return;

    m_maintainAspectRatio = enabled;
    LOG_DEBUG << "Maintain aspect ratio changed to" << (enabled ? "true" : "false");
    update();
}

bool AbstractVideoWidget::maintainAspectRatio() const
{
    return m_maintainAspectRatio;
}

void AbstractVideoWidget::onFrameReady(const FrameHandlePtr& frame)
{
    if (!frame || !frame->isValid()) {
        LOG_WARN << "Received invalid frame";
        m_lastFrame.reset();
        update();
        return;
    }

    m_lastFrame = frame;

    for (const auto& processor : m_processors) {
        if (processor) {
            processor->processFrame(m_lastFrame);
        }
    }

    emit frameUpdated(m_lastFrame);
    update();
}

void AbstractVideoWidget::onProviderError(const QString& message)
{
    LOG_ERROR << "Provider error:" << message;
    emit errorOccurred(message);
}


void AbstractVideoWidget::onProviderStateChanged(IVideoFrameProvider::ProviderState state)
{
    LOG_INFO << "Provider state:" << static_cast<int>(state);
    emit providerStateChanged(state);
}


void AbstractVideoWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter p(this);

    if (!m_lastFrame || !m_lastFrame->isValid()) {
        p.fillRect(rect(), Qt::black);
        return;
    }

    const QImage& img = m_lastFrame->image();
    if (img.isNull()) {
        p.fillRect(rect(), Qt::black);
        return;
    }

    QRect target = rect();

    if (m_maintainAspectRatio && m_aspectRatioMode == Qt::KeepAspectRatio) {
        const QSize imgSize = img.size();
        const double sx = double(target.width()) / imgSize.width();
        const double sy = double(target.height()) / imgSize.height();
        const double s  = std::min(sx, sy);

        const int w = int(imgSize.width() * s);
        const int h = int(imgSize.height() * s);
        const int x = target.x() + (target.width()  - w) / 2;
        const int y = target.y() + (target.height() - h) / 2;

        target = QRect(x, y, w, h);
    }

    p.fillRect(rect(), Qt::black);
    p.drawImage(target, img);
}
