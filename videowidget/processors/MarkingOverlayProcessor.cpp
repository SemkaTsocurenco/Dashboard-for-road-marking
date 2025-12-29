#include "MarkingOverlayProcessor.hpp"
#include "LoggerMacros.hpp"
#include "AppConfig.hpp"
#include <QPen>
#include <QBrush>
#include <QVariant>
#include <QPointF>
#include <QLineF>
#include <QPolygonF>
#include <QPainterPath>
#include <cmath>

using namespace video;

namespace {
    QColor laneColorFromString(const QString& color) {
        const QString c = color.toLower();
        if (c == "white")  return QColor(255, 255, 255);
        if (c == "yellow") return QColor(255, 215, 0);
        if (c == "red")    return QColor(220, 20, 60);
        return QColor(180, 180, 180);
    }

    Qt::PenStyle penStyleForLaneString(const QString& typeStr) {
        const QString t = typeStr.toLower();
        if (t.contains("dashed")) return Qt::DashLine;
        if (t.contains("solid"))  return Qt::SolidLine;
        return Qt::SolidLine;
    }

    QColor objectColorFromStrings(const QString& colorStr, const QString& styleStr,
                                  bool isCrosswalk, bool isArrow) {
        // Class-specific coloring has priority over protocol color for readability
        if (isCrosswalk) return QColor("#00ffff");
        if (isArrow)     return QColor("#ff00ff");

        if (!colorStr.isEmpty() && colorStr != "Unknown") {
            return laneColorFromString(colorStr);
        }

        if (styleStr.toLower() == "double") return QColor("#ffd700");
        return QColor("#4da3ff");
    }

    Qt::PenStyle penStyleFromString(const QString& style) {
        const QString s = style.toLower();
        if (s == "dashed") return Qt::DashLine;
        if (s == "double") return Qt::DotLine;
        return Qt::SolidLine;
    }

    QVector<QPointF> variantPointsToVec(const QVariantList& list) {
        QVector<QPointF> pts;
        pts.reserve(list.size());
        for (const auto& v : list) {
            QPointF p = v.toPointF();
            pts.push_back(p);
        }
        return pts;
    }
}

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

void MarkingOverlayProcessor::updateMarkings(const domain::MarkingObjectModel& model)
{
    if (m_markingObjectListModel) {
        QMutexLocker locker(&m_mutex);
        m_markingObjectListModel->updateFromDomain(model);
        LOG_DEBUG << "MarkingObjectListModel updated from domain, objects count: " << model.size();
    } else {
        LOG_WARN << "Cannot update markings: MarkingObjectListModel not set";
    }
}

void MarkingOverlayProcessor::updateFittedLines(const domain::FittedLinesModel& model)
{
    QMutexLocker locker(&m_mutex);
    m_fittedLinesModel = model;
    LOG_DEBUG << "FittedLinesModel updated, lines count: " << model.size();
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

void MarkingOverlayProcessor::setDrawFittedLines(bool draw)
{
    QMutexLocker locker(&m_mutex);
    m_drawFittedLines = draw;
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

bool MarkingOverlayProcessor::drawFittedLines() const
{
    QMutexLocker locker(&m_mutex);
    return m_drawFittedLines;
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

    if (m_drawFittedLines && m_fittedLinesModel.isValid()) {
        drawFittedLinesOverlay(painter, imageSize);
    }

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

    auto drawBoundaryPoints = [&](const QVariantList& points, const QColor& color){
        const QVector<QPointF> pts = variantPointsToVec(points);
        if (pts.isEmpty())
            return;

        QPen pen(color, 2, Qt::SolidLine);
        painter.setPen(pen);
        painter.setBrush(QBrush(color));

        for (int i = 0; i < pts.size(); ++i) {
            QPointF p = worldToImage(pts[i].x(), pts[i].y(), imageSize);
            painter.drawEllipse(p, 3, 3);
        }
    };

    const QColor leftColor = laneColorFromString(m_laneStateViewModel->laneColorLeft());
    const QColor rightColor = laneColorFromString(m_laneStateViewModel->laneColorRight());

    if (!m_fittedLinesModel.isValid() || m_fittedLinesModel.empty()) {
        drawBoundaryPoints(m_laneStateViewModel->leftPoints(), leftColor);
        drawBoundaryPoints(m_laneStateViewModel->rightPoints(), rightColor);
    }

    // Информационный текст в верхнем левом углу
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10, QFont::Bold));

    // Фон для текста (using config values)
    const auto& overlay_cfg = config::VideoProcessingConfig{}.overlay;
    QRect textBg(overlay_cfg.text_padding_x, overlay_cfg.text_padding_y,
                 overlay_cfg.text_background_width, overlay_cfg.text_background_height);
    painter.fillRect(textBg, QColor(0, 0, 0, 150));

    painter.drawText(10, 20, QString("Lane Width: %1m").arg(m_laneStateViewModel->laneWidthMeters(), 0, 'f', 2));
    painter.drawText(10, 35, QString("Quality: L:%1% R:%2%")
                                  .arg(m_laneStateViewModel->laneQualityLeftPercent())
                                  .arg(m_laneStateViewModel->laneQualityRightPercent()));
    painter.drawText(10, 50, QString("Lines: %1").arg(m_fittedLinesModel.size()));
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

        const int classId = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::ClassIdRole).toInt();
        const float x = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::XMetersRole).toFloat();
        const float y = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::YMetersRole).toFloat();
        const float length = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::LengthMetersRole).toFloat();
        const float width = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::WidthMetersRole).toFloat();
        const float yaw = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::YawDegRole).toFloat();
        const QString className = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::ClassNameRole).toString();
        const bool isCrosswalk = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::IsCrosswalkRole).toBool();
        const bool isArrow = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::IsArrowRole).toBool();
        const float confidence = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::ConfidenceRole).toFloat();
        const QString lineColorStr = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::LineColorRole).toString();
        const QString lineStyleStr = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::LineStyleRole).toString();

        // Get pixel coordinates (V2 protocol)
        const bool hasPixelCoords = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::HasPixelCoordsRole).toBool();
        const float centerXPx = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::CenterXPixelsRole).toFloat();
        const float centerYPx = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::CenterYPixelsRole).toFloat();
        const float widthPx = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::WidthPixelsRole).toFloat();
        const float lengthPx = m_markingObjectListModel->data(index, viewmodels::MarkingObjectListModel::LengthPixelsRole).toFloat();

        QColor color = objectColorFromStrings(lineColorStr, lineStyleStr, isCrosswalk, isArrow);

        // Calculate distance from vehicle (x, y already in meters)
        const float distance = std::sqrt(x * x + y * y);

        // Draw object with contour and size information
        drawMarkingObjectWithContour(painter, imageSize, classId, x, y, length, width, yaw,
                                     className, color, isCrosswalk, isArrow, distance,
                                     hasPixelCoords, centerXPx, centerYPx, widthPx, lengthPx);
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

        const auto& overlay_cfg = config::VideoProcessingConfig{}.overlay;
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

QPointF MarkingOverlayProcessor::worldToImage(float worldX_m, float worldY_m, const QSize& imageSize) const
{
    // GeometryMapper already normalizes detections to decimeters in image space:
    // x_dm = ((cx - w/2) / w) * 100, y_dm = ((h - cy) / h) * 100
    // Parser converts decimeters to meters (value / 10), so here we invert that
    // normalization back to pixel coordinates without any camera intrinsics.

    // Keep image dimensions for completeness (calibration setters remain no-ops for projection)
    const_cast<CameraCalibration&>(m_calibration).imageWidth = imageSize.width();
    const_cast<CameraCalibration&>(m_calibration).imageHeight = imageSize.height();

    const float w = static_cast<float>(imageSize.width());
    const float h = static_cast<float>(imageSize.height());

    // Inverse of GeometryMapper mapping:
    // cx = w/2 + (x_m / 10) * w
    // cy = h - (y_m / 10) * h
    const float img_x = w * 0.5f + (worldX_m / 10.0f) * w;
    const float img_y = h - (worldY_m / 10.0f) * h;

    return QPointF(img_x, img_y);
}

QColor MarkingOverlayProcessor::getColorForLineColor(laneproto::LineColor color) const
{
    switch (color) {
        case laneproto::LineColor::White:
            return QColor(255, 255, 255);
        case laneproto::LineColor::Yellow:
            return QColor(255, 215, 0);
        case laneproto::LineColor::Red:
            return QColor(220, 20, 60);
        case laneproto::LineColor::Unknown:
        default:
            return QColor(180, 180, 180);
    }
}

QColor MarkingOverlayProcessor::getColorForClassId(laneproto::MarkingClassId classId, laneproto::LineColor lineColor) const
{
    // If line has specific color, use it
    if (lineColor != laneproto::LineColor::Unknown) {
        return getColorForLineColor(lineColor);
    }

    // Otherwise color by class type
    switch (classId) {
        case laneproto::MarkingClassId::Crosswalk:
            return QColor(0, 255, 255);  // Cyan
        case laneproto::MarkingClassId::StopLine:
            return QColor(255, 0, 0);  // Red
        case laneproto::MarkingClassId::BoxJunction:
            return QColor(255, 255, 0);  // Yellow
        case laneproto::MarkingClassId::ArrowLeft:
        case laneproto::MarkingClassId::ArrowRight:
        case laneproto::MarkingClassId::ArrowStraight:
        case laneproto::MarkingClassId::ArrowLeftStraight:
        case laneproto::MarkingClassId::ArrowRightStraight:
            return QColor(255, 0, 255);  // Magenta
        case laneproto::MarkingClassId::SolidSingleWhite:
        case laneproto::MarkingClassId::DoubleWhite:
        case laneproto::MarkingClassId::DashedWhite:
            return QColor(255, 255, 255);  // White
        case laneproto::MarkingClassId::SolidSingleYellow:
        case laneproto::MarkingClassId::DoubleYellow:
        case laneproto::MarkingClassId::DashedYellow:
            return QColor(255, 215, 0);  // Yellow
        case laneproto::MarkingClassId::SolidSingleRed:
            return QColor(220, 20, 60);  // Red
        case laneproto::MarkingClassId::MotorIcon:
            return QColor(100, 149, 237);  // Cornflower blue
        case laneproto::MarkingClassId::BikeIcon:
            return QColor(50, 205, 50);  // Lime green
        case laneproto::MarkingClassId::ChannelizingLine:
            return QColor(255, 165, 0);  // Orange
        case laneproto::MarkingClassId::Unknown:
        default:
            return QColor(77, 163, 255);  // Light blue
    }
}

void MarkingOverlayProcessor::drawFittedLinesOverlay(QPainter& painter, const QSize& imageSize)
{
    if (!m_fittedLinesModel.isValid() || m_fittedLinesModel.empty()){
        return;
    }

    LOG_DEBUG << "Drawing " << m_fittedLinesModel.size() << " fitted lines on image size: "
              << imageSize.width() << "x" << imageSize.height();

    const auto& overlay_cfg = config::VideoProcessingConfig{}.overlay;

    for (const auto& line : m_fittedLinesModel.lines()) {

        if (!line.isValid()) {
            LOG_WARN << "Skipping invalid fitted line: y_range=[" << line.yStart() << "," << line.yEnd()
                     << "], poly=[" << line.polyA() << "," << line.polyB() << "," << line.polyC() << "]";
            continue;
        }

        LOG_DEBUG << "Line: y_range=[" << line.yStart() << "," << line.yEnd()
                  << "], poly=[" << line.polyA() << "," << line.polyB() << "," << line.polyC() << "]"
                  << ", conf=" << static_cast<int>(line.confidence())
                  << ", quality=" << static_cast<int>(line.quality());

        auto points = line.generatePoints(overlay_cfg.fitted_line_points_count);
        if (points.empty()) {
            LOG_WARN << "generatePoints returned empty vector";
            continue;
        }

        LOG_DEBUG << "Generated " << points.size() << " points";

        QVector<QPointF> screenPoints;
        screenPoints.reserve(points.size());

        // Detector already provides pixel coordinates in the source frame space
        for (const auto& [x_px, y_px] : points) {
            screenPoints.push_back(QPointF(x_px, y_px));
        }

        if (screenPoints.size() < 2) {
            LOG_WARN << "Not enough screen points: " << screenPoints.size();
            continue;
        }

        LOG_DEBUG << "First point: (" << screenPoints[0].x() << "," << screenPoints[0].y() << ")";
        LOG_DEBUG << "Last point: (" << screenPoints.last().x() << "," << screenPoints.last().y() << ")";

        QColor color = getColorForClassId(line.classId(), line.lineColor());

        LOG_DEBUG << "Drawing line with color: " << color.name().toStdString()
                  << ", classId=" << static_cast<int>(line.classId())
                  << ", lineColor=" << static_cast<int>(line.lineColor())
                  << ", lineStyle=" << static_cast<int>(line.lineStyle());

        // Build a single path so dash pattern spans the whole polyline instead of restarting per segment
        QPainterPath linePath(screenPoints.first());
        for (int i = 1; i < screenPoints.size(); ++i) {
            linePath.lineTo(screenPoints[i]);
        }

        Qt::PenStyle penStyle = Qt::SolidLine;
        int penWidth = 4;

        switch (line.lineStyle()) {
            case laneproto::LineStyle::Solid:
                penStyle = Qt::SolidLine;
                penWidth = 4;
                break;
            case laneproto::LineStyle::Dashed:
                penStyle = Qt::DashLine;
                
                penWidth = 4;
                break;
            case laneproto::LineStyle::Double:
                penStyle = Qt::SolidLine;
                penWidth = 8;
                break;
            case laneproto::LineStyle::Unknown:
            default:
                penStyle = Qt::SolidLine;
                penWidth = 3;
                break;
        }

        LOG_DEBUG << "Using penWidth=" << penWidth << ", penStyle=" << static_cast<int>(penStyle);

        // Disable brush to prevent filling the path interior
        painter.setBrush(Qt::NoBrush);

        // First draw black outline for visibility (always solid regardless of line style)
        QPen outlinePen(Qt::black, penWidth + 2, Qt::SolidLine);
        outlinePen.setCapStyle(Qt::RoundCap);
        outlinePen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(outlinePen);
        painter.setOpacity(overlay_cfg.normal_opacity);

        painter.drawPath(linePath);

        painter.setOpacity(overlay_cfg.full_opacity);

        // Then draw the actual line on top
        QPen pen(color, penWidth, penStyle);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(pen);

        painter.drawPath(linePath);

        // Draw compact coordinate labels at start and end points using V2 data
        const auto& pointsM = line.pointsMeters();
        const auto& pointsPx = line.pointsPixels();

        painter.setFont(QFont("Arial", 8));

        auto drawCompactCoordLabel = [&](const QPointF& pos, float xMeters, float yMeters, const QColor& lineColor) {
            // Draw small point marker
            const float pointRadius = 3.0f;
            painter.setPen(QPen(Qt::black, 1));
            painter.setBrush(lineColor);
            painter.drawEllipse(pos, pointRadius, pointRadius);

            // Label offset
            const float labelOffsetX = 8.0f;
            const float labelOffsetY = -6.0f;
            QPointF labelPos(pos.x() + labelOffsetX, pos.y() + labelOffsetY);

            // Readable label text with clear x/y separation
            QString label = QString("x:%1 y:%2").arg(xMeters, 0, 'f', 1).arg(yMeters, 0, 'f', 1);
            QFontMetrics fm(painter.font());
            QRect textRect = fm.boundingRect(label);
            textRect.moveTopLeft(QPoint(labelPos.x(), labelPos.y() - textRect.height() / 2));
            textRect.adjust(-4, -2, 4, 2);

            // Draw label background with colored border
            painter.setPen(QPen(lineColor, 1.5));
            painter.setBrush(QColor(0, 0, 0, 180));
            painter.drawRoundedRect(textRect, 3, 3);

            // Draw label text
            painter.setPen(Qt::white);
            painter.drawText(textRect, Qt::AlignCenter, label);
        };

        // Draw labels only at start and end points (skip middle to reduce clutter)
        if (pointsPx[0].x_px > 0 || pointsPx[0].y_px > 0) {
            drawCompactCoordLabel(QPointF(pointsPx[0].x_px, pointsPx[0].y_px),
                                  pointsM[0].x_m, pointsM[0].y_m, color);
        }
        if (pointsPx[2].x_px > 0 || pointsPx[2].y_px > 0) {
            drawCompactCoordLabel(QPointF(pointsPx[2].x_px, pointsPx[2].y_px),
                                  pointsM[2].x_m, pointsM[2].y_m, color);
        }

        LOG_DEBUG << "Line drawing completed successfully";
    }
}

void MarkingOverlayProcessor::drawMarkingObjectWithContour(QPainter& painter, const QSize& imageSize,
                                                            int classId,
                                                            float x, float y, float length, float width,
                                                            float yaw, const QString& className,
                                                            const QColor& color, bool isCrosswalk, bool isArrow,
                                                            float distance,
                                                            bool hasPixelCoords,
                                                            float centerXPx, float centerYPx,
                                                            float widthPxParam, float lengthPxParam)
{
    QPointF center;
    float lengthPx;
    float widthPx;

    if (hasPixelCoords && widthPxParam > 0.0f && lengthPxParam > 0.0f) {
        // Use direct pixel coordinates from detector (V2 protocol)
        center = QPointF(centerXPx, centerYPx);
        lengthPx = lengthPxParam;
        widthPx = widthPxParam;
    } else {
        // Fallback: convert from meters using legacy worldToImage
        center = worldToImage(x, y, imageSize);
        // GeometryMapper scales bbox height to image height and width to image width.
        lengthPx = (length / 10.0f) * static_cast<float>(imageSize.height());
        widthPx = (width / 10.0f) * static_cast<float>(imageSize.width());
    }

    painter.save();
    painter.translate(center);
    painter.rotate(-yaw);  // Negative because Qt rotates clockwise

    QRectF rect(-widthPx / 2.0f, -lengthPx / 2.0f, widthPx, lengthPx);

    // Специализированная отрисовка для разных типов
    const auto& overlay_cfg = config::VideoProcessingConfig{}.overlay;

    if (isCrosswalk) {
        // Пешеходный переход - рисуем полоски
        painter.setPen(QPen(color, 2, Qt::SolidLine));
        QColor fillColor = color;
        fillColor.setAlpha(overlay_cfg.shape_alpha_low);
        painter.setBrush(QBrush(fillColor));

        // Рисуем зебру
        int numStripes = 8;
        float stripeWidth = widthPx / numStripes;
        for (int i = 0; i < numStripes; i += 2) {
            QRectF stripe(rect.left() + i * stripeWidth, rect.top(),
                         stripeWidth, lengthPx);
            painter.drawRect(stripe);
        }

        // Контур
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(color, 3, Qt::SolidLine));
        painter.drawRect(rect);

    } else if (isArrow) {
        // For arrows, don't apply yaw rotation here - the arrow type (left/right/straight)
        // already encodes the direction. Just draw at center position.
        // Restore transform first, then draw arrow separately
        painter.restore();
        painter.save();
        painter.translate(center);
        // Draw arrow shape matching detector style (with black outline)
        // Use smaller scale - detector uses fixed template that fits in bbox
        drawArrowShape(painter, classId, widthPx * 0.6f, lengthPx * 0.6f, color);

    } else {
        // Other objects - skip drawing (box_junction, stop_line, etc.)
    }

    painter.restore();

    // Draw coordinate label with small point marker
    painter.setFont(QFont("Arial", 8));

    // Draw small point marker at object center
    const float pointRadius = 3.5f;
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(color);
    painter.drawEllipse(center, pointRadius, pointRadius);

    // Label offset (to the upper right)
    const float labelOffsetX = 10.0f;
    const float labelOffsetY = -lengthPx / 2.0f - 10.0f;
    QPointF labelAnchor(center.x() + labelOffsetX, center.y() + labelOffsetY);

    // Readable label text with clear x/y separation
    QString coordLabel = QString("x:%1 y:%2").arg(x, 0, 'f', 1).arg(y, 0, 'f', 1);
    QFontMetrics fm(painter.font());
    QRect textRect = fm.boundingRect(coordLabel);
    textRect.moveTopLeft(QPoint(labelAnchor.x(), labelAnchor.y() - textRect.height() / 2));
    textRect.adjust(-4, -2, 4, 2);

    // Draw label background with colored border
    painter.setPen(QPen(color, 1.5));
    painter.setBrush(QColor(0, 0, 0, 180));
    painter.drawRoundedRect(textRect, 3, 3);

    // Draw label text
    painter.setPen(Qt::white);
    painter.drawText(textRect, Qt::AlignCenter, coordLabel);
}

void MarkingOverlayProcessor::drawArrowShape(QPainter& painter, int classId, float widthPx, float lengthPx,
                                              const QColor& color)
{
    // Arrow class IDs: 11=left, 12=straight, 13=right, 14=left_straight, 15=right_straight
    // Drawing is done in local coordinates (already translated and rotated)
    // Origin is at center, Y axis points down in Qt

    const float outlineWidth = 3.0f;
    const float arrowWidth = 2.0f;
    const QColor outlineColor = Qt::black;

    // Scale factors based on bounding box
    const float halfW = widthPx / 2.0f;
    const float halfL = lengthPx / 2.0f;

    // Tip size proportional to width
    const float tipSize = widthPx * 0.25f;
    const float shaftWidth = widthPx * 0.15f;

    auto drawStraightArrow = [&](float offsetX = 0.0f, float offsetY = 0.0f, float scale = 1.0f) {
        // Vertical arrow pointing up (in road direction)
        const float tipY = -halfL * scale + offsetY;
        const float tailY = halfL * scale + offsetY;
        const float cx = offsetX;

        // Shaft
        QLineF shaft(cx, tailY, cx, tipY + tipSize * scale);

        // Arrow head triangle
        QPolygonF head;
        head << QPointF(cx, tipY)
             << QPointF(cx - tipSize * scale, tipY + tipSize * scale)
             << QPointF(cx + tipSize * scale, tipY + tipSize * scale);

        // Draw outline first
        painter.setPen(QPen(outlineColor, shaftWidth * scale + outlineWidth, Qt::SolidLine, Qt::RoundCap));
        painter.setBrush(Qt::NoBrush);
        painter.drawLine(shaft);

        painter.setPen(QPen(outlineColor, outlineWidth));
        painter.setBrush(outlineColor);
        painter.drawPolygon(head);

        // Draw colored arrow on top
        painter.setPen(QPen(color, shaftWidth * scale, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(shaft);

        painter.setPen(QPen(color, arrowWidth));
        painter.setBrush(color);
        painter.drawPolygon(head);
    };

    auto drawLeftArrow = [&](float offsetX = 0.0f, float offsetY = 0.0f, float scale = 1.0f) {
        // Horizontal arrow pointing left
        const float tipX = -halfW * scale + offsetX;
        const float tailX = halfW * scale * 0.5f + offsetX;
        const float cy = offsetY;

        // Shaft
        QLineF shaft(tailX, cy, tipX + tipSize * scale, cy);

        // Arrow head triangle
        QPolygonF head;
        head << QPointF(tipX, cy)
             << QPointF(tipX + tipSize * scale, cy - tipSize * scale)
             << QPointF(tipX + tipSize * scale, cy + tipSize * scale);

        // Draw outline first
        painter.setPen(QPen(outlineColor, shaftWidth * scale + outlineWidth, Qt::SolidLine, Qt::RoundCap));
        painter.setBrush(Qt::NoBrush);
        painter.drawLine(shaft);

        painter.setPen(QPen(outlineColor, outlineWidth));
        painter.setBrush(outlineColor);
        painter.drawPolygon(head);

        // Draw colored arrow on top
        painter.setPen(QPen(color, shaftWidth * scale, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(shaft);

        painter.setPen(QPen(color, arrowWidth));
        painter.setBrush(color);
        painter.drawPolygon(head);
    };

    auto drawRightArrow = [&](float offsetX = 0.0f, float offsetY = 0.0f, float scale = 1.0f) {
        // Horizontal arrow pointing right
        const float tipX = halfW * scale + offsetX;
        const float tailX = -halfW * scale * 0.5f + offsetX;
        const float cy = offsetY;

        // Shaft
        QLineF shaft(tailX, cy, tipX - tipSize * scale, cy);

        // Arrow head triangle
        QPolygonF head;
        head << QPointF(tipX, cy)
             << QPointF(tipX - tipSize * scale, cy - tipSize * scale)
             << QPointF(tipX - tipSize * scale, cy + tipSize * scale);

        // Draw outline first
        painter.setPen(QPen(outlineColor, shaftWidth * scale + outlineWidth, Qt::SolidLine, Qt::RoundCap));
        painter.setBrush(Qt::NoBrush);
        painter.drawLine(shaft);

        painter.setPen(QPen(outlineColor, outlineWidth));
        painter.setBrush(outlineColor);
        painter.drawPolygon(head);

        // Draw colored arrow on top
        painter.setPen(QPen(color, shaftWidth * scale, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(shaft);

        painter.setPen(QPen(color, arrowWidth));
        painter.setBrush(color);
        painter.drawPolygon(head);
    };

    switch (classId) {
        case 11:  // ArrowLeft
            drawLeftArrow();
            break;

        case 12:  // ArrowStraight
            drawStraightArrow();
            break;

        case 13:  // ArrowRight
            drawRightArrow();
            break;

        case 14:  // ArrowLeftStraight
            // Combined: straight arrow offset right, left arrow offset up
            drawStraightArrow(halfW * 0.3f, 0.0f, 0.8f);
            drawLeftArrow(0.0f, -halfL * 0.2f, 0.7f);
            break;

        case 15:  // ArrowRightStraight
            // Combined: straight arrow offset left, right arrow offset up
            drawStraightArrow(-halfW * 0.3f, 0.0f, 0.8f);
            drawRightArrow(0.0f, -halfL * 0.2f, 0.7f);
            break;

        default:
            // Unknown arrow type - draw straight arrow as fallback
            drawStraightArrow();
            break;
    }
}
