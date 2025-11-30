#include "LaneStateViewModel.h"
#include <algorithm>

namespace viewmodels {

    LaneStateViewModel::LaneStateViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    void LaneStateViewModel::updateFromDomain(const domain::LaneState& state) {
        bool changed = false;

        // Update validity
        if (valid_ != state.isValid()) {
            valid_ = state.isValid();
            emit validChanged(valid_);
            changed = true;
        }

        if (!valid_) {
            if (changed) {
                reset();
            }
            return;
        }

        // Update lane types
        if (lane_type_left_ != state.laneTypeLeft()) {
            lane_type_left_ = state.laneTypeLeft();
            emit laneTypeLeftChanged();
        }

        if (lane_type_right_ != state.laneTypeRight()) {
            lane_type_right_ = state.laneTypeRight();
            emit laneTypeRightChanged();
        }

        // Update offsets
        if (left_offset_m_ != state.leftOffsetMeters()) {
            left_offset_m_ = state.leftOffsetMeters();
            emit leftOffsetChanged(left_offset_m_);
        }

        if (right_offset_m_ != state.rightOffsetMeters()) {
            right_offset_m_ = state.rightOffsetMeters();
            emit rightOffsetChanged(right_offset_m_);
        }

        // Update calculated values
        float new_width = state.laneWidthMeters();
        if (lane_width_m_ != new_width) {
            lane_width_m_ = new_width;
            emit laneWidthChanged(lane_width_m_);
        }

        float new_center = state.centerOffsetMeters();
        if (center_offset_m_ != new_center) {
            center_offset_m_ = new_center;
            emit centerOffsetChanged(center_offset_m_);
        }

        // Update quality (convert 0-255 to 0-100 percent)
        int new_quality = static_cast<int>((state.qualityRaw() * 100) / 255);
        bool new_quality_good = state.isQualityGood(60); // 60 is default threshold

        if (quality_percent_ != new_quality || is_quality_good_ != new_quality_good) {
            quality_percent_ = new_quality;
            is_quality_good_ = new_quality_good;
            emit qualityChanged();
        }

        // Update timestamp
        if (timestamp_ms_ != state.timestampMs()) {
            timestamp_ms_ = state.timestampMs();
            emit timestampChanged(timestamp_ms_);
        }

        // Details block
        bool new_has_details = state.hasDetails();
        if (has_details_ != new_has_details) {
            has_details_ = new_has_details;
            emit detailsChanged(has_details_);
        }

        QString new_color_left = lineColorToString(state.laneColorLeft());
        if (lane_color_left_ != new_color_left) {
            lane_color_left_ = new_color_left;
            emit laneColorLeftChanged();
        }

        QString new_color_right = lineColorToString(state.laneColorRight());
        if (lane_color_right_ != new_color_right) {
            lane_color_right_ = new_color_right;
            emit laneColorRightChanged();
        }

        float new_width_left = state.laneWidthLeftMeters();
        if (lane_width_left_m_ != new_width_left) {
            lane_width_left_m_ = new_width_left;
            emit laneWidthLeftChanged(lane_width_left_m_);
        }

        float new_width_right = state.laneWidthRightMeters();
        if (lane_width_right_m_ != new_width_right) {
            lane_width_right_m_ = new_width_right;
            emit laneWidthRightChanged(lane_width_right_m_);
        }

        int new_quality_left = static_cast<int>((state.laneQualityLeftRaw() * 100) / 255);
        if (lane_quality_left_percent_ != new_quality_left) {
            lane_quality_left_percent_ = new_quality_left;
            emit laneQualityLeftChanged(lane_quality_left_percent_);
        }

        int new_quality_right = static_cast<int>((state.laneQualityRightRaw() * 100) / 255);
        if (lane_quality_right_percent_ != new_quality_right) {
            lane_quality_right_percent_ = new_quality_right;
            emit laneQualityRightChanged(lane_quality_right_percent_);
        }

        QVariantList new_left_points = toVariantPoints(state.leftPoints(), state.leftPointsCount());
        QVariantList new_right_points = toVariantPoints(state.rightPoints(), state.rightPointsCount());
        if (left_points_ != new_left_points || right_points_ != new_right_points) {
            left_points_ = new_left_points;
            right_points_ = new_right_points;
            emit lanePointsChanged();
        }
    }

    void LaneStateViewModel::reset() {
        if (valid_) {
            valid_ = false;
            emit validChanged(false);
        }

        lane_type_left_ = {};
        lane_type_right_ = {};
        left_offset_m_ = 0.0f;
        right_offset_m_ = 0.0f;
        lane_width_m_ = 0.0f;
        center_offset_m_ = 0.0f;
        quality_percent_ = 0;
        is_quality_good_ = false;
        timestamp_ms_ = 0;
        has_details_ = false;
        lane_color_left_ = QStringLiteral("Unknown");
        lane_color_right_ = QStringLiteral("Unknown");
        lane_width_left_m_ = 0.0f;
        lane_width_right_m_ = 0.0f;
        lane_quality_left_percent_ = 0;
        lane_quality_right_percent_ = 0;
        left_points_.clear();
        right_points_.clear();

        emit laneTypeLeftChanged();
        emit laneTypeRightChanged();
        emit leftOffsetChanged(0.0f);
        emit rightOffsetChanged(0.0f);
        emit laneWidthChanged(0.0f);
        emit centerOffsetChanged(0.0f);
        emit qualityChanged();
        emit timestampChanged(0);
        emit detailsChanged(false);
        emit laneColorLeftChanged();
        emit laneColorRightChanged();
        emit laneWidthLeftChanged(0.0f);
        emit laneWidthRightChanged(0.0f);
        emit laneQualityLeftChanged(0);
        emit laneQualityRightChanged(0);
        emit lanePointsChanged();
    }

    QString LaneStateViewModel::laneTypeLeft() const {
        return laneTypeToString(lane_type_left_);
    }

    QString LaneStateViewModel::laneTypeRight() const {
        return laneTypeToString(lane_type_right_);
    }

    QString LaneStateViewModel::laneTypeToString(laneproto::LaneType type) const {
        switch (type) {
            case laneproto::LaneType::Solid:
                return QStringLiteral("Solid");
            case laneproto::LaneType::Dashed:
                return QStringLiteral("Dashed");
            case laneproto::LaneType::DoubleSolid:
                return QStringLiteral("DoubleSolid");
            case laneproto::LaneType::DoubleDashed:
                return QStringLiteral("DoubleDashed");
            case laneproto::LaneType::SolidDashed:
                return QStringLiteral("SolidDashed");
            case laneproto::LaneType::Unknown:
            default:
                return QStringLiteral("Unknown");
        }
    }

    QString LaneStateViewModel::lineColorToString(laneproto::LineColor color) const {
        switch (color) {
            case laneproto::LineColor::White:
                return QStringLiteral("White");
            case laneproto::LineColor::Yellow:
                return QStringLiteral("Yellow");
            case laneproto::LineColor::Red:
                return QStringLiteral("Red");
            case laneproto::LineColor::Unknown:
            default:
                return QStringLiteral("Unknown");
        }
    }

    QVariantList LaneStateViewModel::toVariantPoints(const std::array<laneproto::LanePoint, 3>& points,
                                                     std::uint8_t count) const {
        QVariantList list;
        count = std::min<std::uint8_t>(count, 3);
        list.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            const auto& p = points[i];
            list.push_back(QVariant::fromValue(QPointF(p.x_m, p.y_m)));
        }
        return list;
    }

}
