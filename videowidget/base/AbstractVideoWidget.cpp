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

void AbstractVideoWidget::pause()
{
    if (!m_provider) {
        LOG_WARN << "Cannot pause: provider is null";
        return;
    }
    LOG_INFO << "Pausing provider";
    m_provider->pause();
}

void AbstractVideoWidget::resume()
{
    if (!m_provider) {
        LOG_WARN << "Cannot resume: provider is null";
        return;
    }
    LOG_INFO << "Resuming provider";
    m_provider->resume();
}

bool AbstractVideoWidget::isRunning() const
{
    return m_provider ? m_provider->isRunning() : false;
}

bool AbstractVideoWidget::isPaused() const
{
    return m_provider ? (m_provider->state() == IVideoFrameProvider::ProviderState::Paused) : false;
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

void AbstractVideoWidget::removeFrameProcessor(const FrameProcessorPtr& processor)
{
    if (!processor) {
        LOG_WARN << "Tried to remove null frame processor";
        return;
    }
    auto it = std::find(m_processors.begin(), m_processors.end(), processor);
    if (it != m_processors.end()) {
        m_processors.erase(it);
        LOG_DEBUG << "Removed frame processor, count =" << m_processors.size();
    } else {
        LOG_WARN << "Frame processor not found";
    }
}

void AbstractVideoWidget::clearFrameProcessors()
{
    m_processors.clear();
    LOG_INFO << "Frame processors cleared";
}

int AbstractVideoWidget::processorCount() const
{
    return m_processors.size();
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

void AbstractVideoWidget::setBackgroundColor(const QColor& color)
{
    if (m_backgroundColor == color)
        return;

    m_backgroundColor = color;
    LOG_DEBUG << "Background color changed";
    update();
}

QColor AbstractVideoWidget::backgroundColor() const
{
    return m_backgroundColor;
}

void AbstractVideoWidget::setShowFps(bool show)
{
    if (m_showFps == show)
        return;

    m_showFps = show;
    if (m_showFps) {
        m_fpsTimer.start();
        m_frameCounter = 0;
    }
    LOG_DEBUG << "Show FPS changed to" << (show ? "true" : "false");
    update();
}

bool AbstractVideoWidget::showFps() const
{
    return m_showFps;
}

double AbstractVideoWidget::currentFps() const
{
    return m_currentFps;
}

int64_t AbstractVideoWidget::framesProcessed() const
{
    return m_totalFrames;
}

void AbstractVideoWidget::resetStatistics()
{
    m_totalFrames = 0;
    m_frameCounter = 0;
    m_currentFps = 0.0;
    if (m_showFps) {
        m_fpsTimer.restart();
    }
    LOG_INFO << "Statistics reset";
}

QImage AbstractVideoWidget::captureFrame() const
{
    return lastFrameImage();
}

bool AbstractVideoWidget::saveFrame(const QString& filePath) const
{
    QImage img = captureFrame();
    if (img.isNull()) {
        LOG_WARN << "Cannot save null image";
        return false;
    }

    bool result = img.save(filePath);
    if (result) {
        LOG_INFO << "Frame saved to" << filePath.toStdString();
    } else {
        LOG_ERROR << "Failed to save frame to" << filePath.toStdString();
    }
    return result;
}

void AbstractVideoWidget::updateFpsCounter()
{
    if (!m_showFps)
        return;

    ++m_frameCounter;
    qint64 elapsed = m_fpsTimer.elapsed();

    if (elapsed >= 1000) {
        m_currentFps = (m_frameCounter * 1000.0) / elapsed;
        emit fpsChanged(m_currentFps);
        m_frameCounter = 0;
        m_fpsTimer.restart();
    }
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
    ++m_totalFrames;
    updateFpsCounter();

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
    LOG_ERROR << "Provider error:" << message.toStdString();
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
        p.fillRect(rect(), m_backgroundColor);

        p.setPen(Qt::gray);
        p.setFont(QFont("Arial", 24, QFont::Bold));
        p.drawText(rect(), Qt::AlignCenter, "NO VIDEO");

        drawOverlay(p);
        return;
    }

    const QImage& img = m_lastFrame->image();
    if (img.isNull()) {
        p.fillRect(rect(), m_backgroundColor);

        p.setPen(Qt::gray);
        p.setFont(QFont("Arial", 24, QFont::Bold));
        p.drawText(rect(), Qt::AlignCenter, "NO VIDEO");

        drawOverlay(p);
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

    p.fillRect(rect(), m_backgroundColor);
    p.drawImage(target, img);
    drawOverlay(p);
}

void AbstractVideoWidget::drawOverlay(QPainter& painter)
{
    if (!m_showFps)
        return;

    painter.save();
    painter.setPen(Qt::green);
    painter.setFont(QFont("Arial", 12, QFont::Bold));

    QString fpsText = QString("FPS: %1").arg(m_currentFps, 0, 'f', 1);
    QString framesText = QString("Frames: %1").arg(m_totalFrames);

    painter.drawText(10, 20, fpsText);
    painter.drawText(10, 40, framesText);
    painter.restore();
}
