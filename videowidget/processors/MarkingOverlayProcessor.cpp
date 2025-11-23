#include "MarkingOverlayProcessor.hpp"
#include "LoggerMacros.hpp"
#include <QPen>
#include <QBrush>
#include <cmath>

using namespace video;

MarkingOverlayProcessor::MarkingOverlayProcessor()
{
    LOG_TRACE << "MarkingOverlayProcessor created";
}

void MarkingOverlayProcessor::setLaneStateViewModel(viewmodels::LaneStateViewModel* viewModel)
{
    QMutexLocker locker(&m_mutex);
    m_laneStateViewModel = viewModel;
    LOG_DEBUG << "LaneStateViewModel set";
}

void MarkingOverlayProcessor::setMarkingObjectListModel(viewmodels::MarkingObjectListModel* model)
{
    QMutexLocker locker(&m_mutex);
    m_markingObjectListModel = model;
    LOG_DEBUG << "MarkingObjectListModel set";
}

void MarkingOverlayProcessor::setWarningListModel(viewmodels::WarningListModel* model)
{
    QMutexLocker locker(&m_mutex);
    m_warningListModel = model;
    LOG_DEBUG << "WarningListModel set";
}

void MarkingOverlayProcessor::setDrawLanes(bool draw)
{
    QMutexLocker locker(&m_mutex);
    m_drawLanes = draw;
}

void MarkingOverlayProcessor::setDrawMarkings(bool draw)
{
    QMutexLocker locker(&m_mutex);
    m_drawMarkings = draw;
}

void MarkingOverlayProcessor::setDrawWarnings(bool draw)
{
    QMutexLocker locker(&m_mutex);
    m_drawWarnings = draw;
}

bool MarkingOverlayProcessor::drawLanes() const
{
    QMutexLocker locker(&m_mutex);
    return m_drawLanes;
}

bool MarkingOverlayProcessor::drawMarkings() const
{
    QMutexLocker locker(&m_mutex);
    return m_drawMarkings;
}

bool MarkingOverlayProcessor::drawWarnings() const
{
    QMutexLocker locker(&m_mutex);
    return m_drawWarnings;
}

void MarkingOverlayProcessor::processFrame(const FrameHandlePtr& frame)
{
    if (!frame || !frame->isValid()) {
        LOG_WARN << "Invalid frame received";
        return;
    }

    if (!m_enabled) {
        return;
    }

    QMutexLocker locker(&m_mutex);
    m_processing = true;

    QImage& image = frame->writableImage();
    drawOverlay(image);

    m_processing = false;
}

void MarkingOverlayProcessor::processFrameAsync(const FrameHandlePtr& frame, ProcessingCallback callback)
{
    if (!frame || !frame->isValid()) {
        LOG_WARN << "Invalid frame received for async processing";
        if (callback) {
            callback(false, "Invalid frame");
        }
        return;
    }

    if (!m_enabled) {
        if (callback) {
            callback(true, "");
        }
        return;
    }

    try {
        processFrame(frame);
        if (callback) {
            callback(true, "");
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "Exception in async processing:" << e.what();
        if (callback) {
            callback(false, QString::fromStdString(e.what()));
        }
    }
}

bool MarkingOverlayProcessor::isProcessing() const
{
    QMutexLocker locker(&m_mutex);
    return m_processing;
}

void MarkingOverlayProcessor::cancel()
{
    QMutexLocker locker(&m_mutex);
    m_processing = false;
    LOG_DEBUG << "Processing cancelled";
}

QString MarkingOverlayProcessor::name() const
{
    return "MarkingOverlayProcessor";
}

void MarkingOverlayProcessor::reset()
{
    QMutexLocker locker(&m_mutex);
    m_processing = false;
    LOG_INFO << "Processor reset";
}

void MarkingOverlayProcessor::setEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    if (m_enabled == enabled)
        return;

    m_enabled = enabled;
    LOG_INFO << "Processor enabled:" << (enabled ? "true" : "false");
}

bool MarkingOverlayProcessor::isEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_enabled;
}

void MarkingOverlayProcessor::drawOverlay(QImage& image)
{
    if (image.isNull())
        return;

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    const QSize imageSize = image.size();

    if (m_drawLanes && m_laneStateViewModel) {
        drawLaneOverlay(painter, imageSize);
    }

    if (m_drawMarkings && m_markingObjectListModel) {
        drawMarkingObjects(painter, imageSize);
    }

    if (m_drawWarnings && m_warningListModel) {
        drawWarnings(painter, imageSize);
    }
}

void MarkingOverlayProcessor::drawLaneOverlay(QPainter& painter, const QSize& imageSize)
{
    if (!m_laneStateViewModel || !m_laneStateViewModel->isValid())
        return;

    const int centerX = imageSize.width() / 2;
    const int bottomY = imageSize.height();
    const int topY = imageSize.height() / 2;

    const float leftOffset = m_laneStateViewModel->leftOffsetMeters();
    const float rightOffset = m_laneStateViewModel->rightOffsetMeters();
    const float centerOffset = m_laneStateViewModel->centerOffsetMeters();

    const float pixelsPerMeter = imageSize.width() / 10.0f;

    const int leftLineX = centerX + static_cast<int>(leftOffset * pixelsPerMeter);
    const int rightLineX = centerX + static_cast<int>(rightOffset * pixelsPerMeter);
    const int centerLineX = centerX + static_cast<int>(centerOffset * pixelsPerMeter);

    QPen lanePen(Qt::green, 3);
    painter.setPen(lanePen);

    painter.drawLine(leftLineX, bottomY, leftLineX, topY);
    painter.drawLine(rightLineX, bottomY, rightLineX, topY);

    QPen centerPen(Qt::yellow, 2, Qt::DashLine);
    painter.setPen(centerPen);
    painter.drawLine(centerLineX, bottomY, centerLineX, topY);

    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10));
    painter.drawText(10, 20, QString("Lane Width: %1m").arg(m_laneStateViewModel->laneWidthMeters(), 0, 'f', 2));
    painter.drawText(10, 35, QString("Quality: %1%").arg(m_laneStateViewModel->qualityPercent()));
}

void MarkingOverlayProcessor::drawMarkingObjects(QPainter& painter, const QSize& imageSize)
{
    if (!m_markingObjectListModel)
        return;

    const int count = m_markingObjectListModel->rowCount();
    if (count == 0)
        return;

    for (int i = 0; i < count; ++i) {
        QModelIndex index = m_markingObjectListModel->index(i, 0);

        const float x = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::XMetersRole).toFloat();
        const float y = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::YMetersRole).toFloat();
        const QString className = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::ClassNameRole).toString();
        const bool isCrosswalk = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::IsCrosswalkRole).toBool();
        const bool isArrow = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::IsArrowRole).toBool();
        const float confidence = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::ConfidenceRole).toFloat();

        QPointF pos = worldToImage(x, y, imageSize);

        QColor color = isCrosswalk ? Qt::cyan : (isArrow ? Qt::magenta : Qt::blue);
        const int radius = 8;

        painter.setPen(QPen(color, 2));
        painter.setBrush(QBrush(color, Qt::SolidPattern));
        painter.drawEllipse(pos, radius, radius);

        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 8));
        painter.drawText(pos.x() + radius + 2, pos.y(), QString("%1 (%.0f%%)").arg(className).arg(confidence * 100));
    }
}

void MarkingOverlayProcessor::drawWarnings(QPainter& painter, const QSize& imageSize)
{
    if (!m_warningListModel)
        return;

    const int count = m_warningListModel->rowCount();
    if (count == 0)
        return;

    int yOffset = 50;
    painter.setFont(QFont("Arial", 11, QFont::Bold));

    for (int i = 0; i < count; ++i) {
        QModelIndex index = m_warningListModel->index(i, 0);

        const bool isActive = m_warningListModel->data(index, viewmodels::WarningListModel::IsActiveRole).toBool();
        if (!isActive)
            continue;

        const bool isCritical = m_warningListModel->data(index, viewmodels::WarningListModel::IsCriticalRole).toBool();
        const QString message = m_warningListModel->data(index, viewmodels::WarningListModel::MessageRole).toString();
        const float distance = m_warningListModel->data(index, viewmodels::WarningListModel::DistanceMetersRole).toFloat();

        QColor bgColor = isCritical ? QColor(220, 0, 0, 180) : QColor(255, 165, 0, 180);
        QColor textColor = Qt::white;

        QString text = QString("%1 (%.1fm)").arg(message).arg(distance);
        QFontMetrics fm(painter.font());
        QRect textRect = fm.boundingRect(text);
        textRect.adjust(-5, -3, 5, 3);
        textRect.moveTopLeft(QPoint(10, yOffset));

        painter.fillRect(textRect, bgColor);
        painter.setPen(textColor);
        painter.drawText(textRect, Qt::AlignCenter, text);

        yOffset += textRect.height() + 5;
    }
}

QPointF MarkingOverlayProcessor::worldToImage(float x, float y, const QSize& imageSize) const
{
    const float pixelsPerMeter = imageSize.width() / 10.0f;
    const int centerX = imageSize.width() / 2;
    const int bottomY = imageSize.height();

    const int imgX = centerX + static_cast<int>(y * pixelsPerMeter);
    const int imgY = bottomY - static_cast<int>(x * pixelsPerMeter);

    return QPointF(imgX, imgY);
}
