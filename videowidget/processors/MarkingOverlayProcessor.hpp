#pragma once

#include "IVideoFrameProcessor.hpp"
#include "LaneStateViewModel.h"
#include "MarkingObjectListModel.h"
#include "WarningListModel.h"
#include "MarkingObject.h"
#include "FittedLine.h"
#include <QPainter>
#include <QMutex>

namespace video {

    class MarkingOverlayProcessor : public IVideoFrameProcessor
    {
    public:
        MarkingOverlayProcessor();
        ~MarkingOverlayProcessor() override = default;

        void processFrame(const FrameHandlePtr& frame) override;
        void processFrameAsync(const FrameHandlePtr& frame, ProcessingCallback callback) override;
        [[nodiscard]] bool isProcessing() const override;
        void cancel() override;
        [[nodiscard]] QString name() const override;
        void reset() override;

        void setEnabled(bool enabled);
        [[nodiscard]] bool isEnabled() const;

        void setLaneStateViewModel(viewmodels::LaneStateViewModel* viewModel);
        void setMarkingObjectListModel(viewmodels::MarkingObjectListModel* model);
        void setWarningListModel(viewmodels::WarningListModel* model);

        // Update marking data from domain model
        void updateMarkings(const domain::MarkingObjectModel& model);
        void updateFittedLines(const domain::FittedLinesModel& model);

        void setDrawLanes(bool draw);
        void setDrawMarkings(bool draw);
        void setDrawWarnings(bool draw);
        void setDrawFittedLines(bool draw);

        [[nodiscard]] bool drawLanes() const;
        [[nodiscard]] bool drawMarkings() const;
        [[nodiscard]] bool drawWarnings() const;
        [[nodiscard]] bool drawFittedLines() const;

    private:
        bool m_enabled = true;
        bool m_processing = false;
        mutable QMutex m_mutex;

        viewmodels::LaneStateViewModel* m_laneStateViewModel = nullptr;
        viewmodels::MarkingObjectListModel* m_markingObjectListModel = nullptr;
        viewmodels::WarningListModel* m_warningListModel = nullptr;

        // Store fitted lines model
        domain::FittedLinesModel m_fittedLinesModel;

        bool m_drawLanes = true;
        bool m_drawMarkings = true;
        bool m_drawWarnings = true;
        bool m_drawFittedLines = true;

        // Legacy camera calibration (kept for API compatibility, not used in projection)
        struct CameraCalibration {
            float focalLengthPixels = 4500.0f;  // Focal length in pixels (typical for 1080p)
            float cameraHeightMeters = 1.0f;   // Camera height above road
            float cameraPitchDeg = 0.0f;      // Camera pitch (negative = looking down)
            float metersPerPixelAtGround = 0.0f; // Scale factor for ground plane
            int imageWidth = 1920;
            int imageHeight = 1080;
        } m_calibration;

        void drawOverlay(QImage& image);
        void drawLaneOverlay(QPainter& painter, const QSize& imageSize);
        void drawMarkingObjects(QPainter& painter, const QSize& imageSize);
        void drawFittedLinesOverlay(QPainter& painter, const QSize& imageSize);
        void drawWarnings(QPainter& painter, const QSize& imageSize);

        void drawMarkingObjectWithContour(QPainter& painter, const QSize& imageSize,
                                         int classId,
                                         float x, float y, float length, float width,
                                         float yaw, const QString& className,
                                         const QColor& color, bool isCrosswalk, bool isArrow,
                                         float distance,
                                         bool hasPixelCoords = false,
                                         float centerXPx = 0.0f, float centerYPx = 0.0f,
                                         float widthPx = 0.0f, float lengthPx = 0.0f);

        // Arrow drawing helper methods (matching detector style)
        void drawArrowShape(QPainter& painter, int classId, float widthPx, float lengthPx,
                           const QColor& color);

        QPointF worldToImage(float x, float y, const QSize& imageSize) const;
        QColor getColorForClassId(laneproto::MarkingClassId classId, laneproto::LineColor lineColor) const;
        QColor getColorForLineColor(laneproto::LineColor color) const;

    public:
        // Camera calibration setters
        void setCameraFocalLength(float focalLength) { m_calibration.focalLengthPixels = focalLength; }
        void setCameraHeight(float height) { m_calibration.cameraHeightMeters = height; }
        void setCameraPitch(float pitchDeg) { m_calibration.cameraPitchDeg = pitchDeg; }
        void setMetersPerPixel(float scale) { m_calibration.metersPerPixelAtGround = scale; }
    };

} // namespace video
