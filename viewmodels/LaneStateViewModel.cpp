#include "LaneStateViewModel.h"

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

        emit laneTypeLeftChanged();
        emit laneTypeRightChanged();
        emit leftOffsetChanged(0.0f);
        emit rightOffsetChanged(0.0f);
        emit laneWidthChanged(0.0f);
        emit centerOffsetChanged(0.0f);
        emit qualityChanged();
        emit timestampChanged(0);
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

}
