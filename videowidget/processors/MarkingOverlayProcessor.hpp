#pragma once

#include "IVideoFrameProcessor.hpp"
#include "LaneStateViewModel.h"
#include "MarkingObjectListModel.h"
#include "WarningListModel.h"
#include "MarkingObject.h"
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

        void setDrawLanes(bool draw);
        void setDrawMarkings(bool draw);
        void setDrawWarnings(bool draw);

        [[nodiscard]] bool drawLanes() const;
        [[nodiscard]] bool drawMarkings() const;
        [[nodiscard]] bool drawWarnings() const;

    private:
        bool m_enabled = true;
        bool m_processing = false;
        mutable QMutex m_mutex;

        viewmodels::LaneStateViewModel* m_laneStateViewModel = nullptr;
        viewmodels::MarkingObjectListModel* m_markingObjectListModel = nullptr;
        viewmodels::WarningListModel* m_warningListModel = nullptr;

        bool m_drawLanes = true;
        bool m_drawMarkings = true;
        bool m_drawWarnings = true;

        void drawOverlay(QImage& image);
        void drawLaneOverlay(QPainter& painter, const QSize& imageSize);
        void drawMarkingObjects(QPainter& painter, const QSize& imageSize);
        void drawWarnings(QPainter& painter, const QSize& imageSize);

        QPointF worldToImage(float x, float y, const QSize& imageSize) const;
    };

} // namespace video
