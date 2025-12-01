#include "MarkingOverlayProcessor.hpp"
#include "LoggerMacros.hpp"
#include <QPen>
#include <QBrush>
#include <QVariant>
#include <QPointF>

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
        if (!colorStr.isEmpty() && colorStr != "Unknown") {
            return laneColorFromString(colorStr);
        }
        if (isCrosswalk) return QColor("#00ffff");
        if (isArrow)     return QColor("#ff00ff");
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

    // Фон для текста
    QRect textBg(5, 5, 250, 50);
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

        QColor color = objectColorFromStrings(lineColorStr, lineStyleStr, isCrosswalk, isArrow);

        // Draw object with contour and size information
        drawMarkingObjectWithContour(painter, imageSize, x, y, length, width, yaw,
                                     className, color, isCrosswalk, isArrow);
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

    // Python sends coordinates for 640x480 image, scale to current size
    const float scaleX = static_cast<float>(imageSize.width()) / 640.0f;
    const float scaleY = static_cast<float>(imageSize.height()) / 480.0f;

    LOG_DEBUG << "Scaling factors: scaleX=" << scaleX << ", scaleY=" << scaleY;

    for (const auto& line : m_fittedLinesModel.lines()) {

        if (!line.isValid() || !line.isConfident(50)) {
            LOG_DEBUG << "Skipping invalid or low confidence line";
            continue;
        }

        LOG_DEBUG << "Line: y_range=[" << line.yStart() << "," << line.yEnd()
                  << "], poly=[" << line.polyA() << "," << line.polyB() << "," << line.polyC() << "]";

        auto points = line.generatePoints(100);
        if (points.empty()) {
            LOG_WARN << "generatePoints returned empty vector";
            continue;
        }

        LOG_DEBUG << "Generated " << points.size() << " points";

        QVector<QPointF> screenPoints;
        screenPoints.reserve(points.size());

        for (const auto& [x_px, y_px] : points) {
            // Scale from 640x480 to actual image size
            float scaled_x = x_px * scaleX;
            float scaled_y = y_px * scaleY;
            screenPoints.push_back(QPointF(scaled_x, scaled_y));
        }

        if (screenPoints.size() < 2) {
            LOG_WARN << "Not enough screen points: " << screenPoints.size();
            continue;
        }

        LOG_DEBUG << "First point (scaled): (" << screenPoints[0].x() << "," << screenPoints[0].y() << ")";
        LOG_DEBUG << "Last point (scaled): (" << screenPoints.last().x() << "," << screenPoints.last().y() << ")";

        QColor color = getColorForClassId(line.classId(), line.lineColor());

        LOG_DEBUG << "Drawing line with color: " << color.name().toStdString()
                  << ", classId=" << static_cast<int>(line.classId())
                  << ", lineColor=" << static_cast<int>(line.lineColor())
                  << ", lineStyle=" << static_cast<int>(line.lineStyle());

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

        // First draw black outline for visibility
        QPen outlinePen(Qt::black, penWidth + 2, penStyle);
        outlinePen.setCapStyle(Qt::RoundCap);
        outlinePen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(outlinePen);
        painter.setOpacity(0.5);

        for (int i = 1; i < screenPoints.size(); ++i) {
            painter.drawLine(screenPoints[i-1], screenPoints[i]);
        }

        painter.setOpacity(1.0);

        // Then draw the actual line on top
        QPen pen(color, penWidth, penStyle);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(pen);

        for (int i = 1; i < screenPoints.size(); ++i) {
            painter.drawLine(screenPoints[i-1], screenPoints[i]);
        }

        LOG_DEBUG << "Line drawing completed successfully";
    }
}

void MarkingOverlayProcessor::drawMarkingObjectWithContour(QPainter& painter, const QSize& imageSize,
                                                            float x, float y, float length, float width,
                                                            float yaw, const QString& className,
                                                            const QColor& color, bool isCrosswalk, bool isArrow)
{
    QPointF center = worldToImage(x, y, imageSize);

    // GeometryMapper scales bbox height to image height and width to image width.
    const float lengthPx = (length / 10.0f) * static_cast<float>(imageSize.height());
    const float widthPx = (width / 10.0f) * static_cast<float>(imageSize.width());

    painter.save();
    painter.translate(center);
    painter.rotate(-yaw);  // Negative because Qt rotates clockwise

    QRectF rect(-widthPx / 2.0f, -lengthPx / 2.0f, widthPx, lengthPx);

    // Специализированная отрисовка для разных типов
    if (isCrosswalk) {
        // Пешеходный переход - рисуем полоски
        painter.setPen(QPen(color, 2, Qt::SolidLine));
        QColor fillColor = color;
        fillColor.setAlpha(100);
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
        // Стрелка - рисуем упрощенную форму стрелки
        painter.setPen(QPen(color, 3, Qt::SolidLine));
        QColor fillColor = color;
        fillColor.setAlpha(120);
        painter.setBrush(QBrush(fillColor));

        // Тело стрелки
        float shaftWidth = widthPx * 0.4f;
        float shaftLength = lengthPx * 0.6f;
        QRectF shaft(-shaftWidth/2, -lengthPx/2, shaftWidth, shaftLength);
        painter.drawRect(shaft);

        // Голова стрелки (треугольник)
        QPolygonF arrowHead;
        float headY = -lengthPx/2 + shaftLength;
        arrowHead << QPointF(0, lengthPx/2)  // Вершина
                  << QPointF(-widthPx/2, headY)  // Левый угол
                  << QPointF(widthPx/2, headY);  // Правый угол
        painter.drawPolygon(arrowHead);

    } else {
        // Обычный объект - прямоугольник
        painter.setPen(QPen(color, 2, Qt::SolidLine));
        QColor fillColor = color;
        fillColor.setAlpha(80);
        painter.setBrush(QBrush(fillColor));
        painter.drawRect(rect);

        // Толстый контур
        painter.setPen(QPen(color, 3, Qt::SolidLine));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(rect);
    }

    painter.restore();

    // Центральная точка
    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(QBrush(color));
    painter.drawEllipse(center, 4, 4);

    // Метка с названием
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 9, QFont::Bold));

    // Фон для текста
    QString label = className;
    QFontMetrics fm(painter.font());
    QRect textRect = fm.boundingRect(label);
    textRect.moveCenter(QPoint(center.x() + textRect.width()/2 + 10, center.y() - 10));
    textRect.adjust(-3, -1, 3, 1);

    painter.fillRect(textRect, QColor(0, 0, 0, 180));
    painter.drawText(textRect, Qt::AlignCenter, label);
}
