#include "LaneState.h"
#include <ostream>
#include <cmath>

namespace domain {

    laneproto::LaneType LaneState::laneTypeLeft() const noexcept {
        return lane_type_left_;
    }

    laneproto::LaneType LaneState::laneTypeRight() const noexcept {
        return lane_type_right_;
    }

    float LaneState::leftOffsetMeters() const noexcept {
        return left_offset_m_;
    }

    float LaneState::rightOffsetMeters() const noexcept {
        return right_offset_m_;
    }

    std::uint8_t LaneState::rawAllowedManeuvers() const noexcept {
        return allowed_maneuvers_raw_;
    }

    std::uint8_t LaneState::qualityRaw() const noexcept {
        return quality_raw_;
    }

    laneproto::TimestampMs LaneState::timestampMs() const noexcept {
        return timestamp_ms_;
    }

    laneproto::SequenceNumber LaneState::seq() const noexcept {
        return seq_;
    }

    bool LaneState::isValid() const noexcept {
        return valid_;
    }

    void LaneState::updateFromProto(const laneproto::LaneSummary& msg) noexcept {
        timestamp_ms_ = msg.timestamp_ms;
        seq_ = msg.seq;
        left_offset_m_ = msg.left_offset_m;
        right_offset_m_ = msg.right_offset_m;
        lane_type_left_ = msg.lane_type_left;
        lane_type_right_ = msg.lane_type_right;
        allowed_maneuvers_raw_ = msg.allowed_maneuvers;
        quality_raw_ = msg.quality;

        valid_ = true;
    }

    void LaneState::reset() noexcept {
        lane_type_left_ = {};
        lane_type_right_ = {};
        left_offset_m_ = 0.0f;
        right_offset_m_ = 0.0f;
        allowed_maneuvers_raw_ = 0;
        quality_raw_ = 0;
        timestamp_ms_ = {};
        seq_ = {};
        valid_ = false;
    }

    bool LaneState::isQualityGood(std::uint8_t threshold) const noexcept {
        return valid_ && quality_raw_ >= threshold;
    }

    float LaneState::laneWidthMeters() const noexcept {
        return std::fabs(right_offset_m_ - left_offset_m_);
    }

    float LaneState::centerOffsetMeters() const noexcept {
        return (left_offset_m_ + right_offset_m_) * 0.5f;
    }

    bool LaneState::isManeuverAllowed(std::uint8_t maneuver_bit) const noexcept {
        return valid_ && (allowed_maneuvers_raw_ & (1u << maneuver_bit)) != 0;
    }

    bool LaneState::hasValidOffsets() const noexcept {
        return valid_ && std::isfinite(left_offset_m_) && std::isfinite(right_offset_m_)
               && left_offset_m_ < right_offset_m_;
    }

    bool LaneState::isSymmetric(float tolerance) const noexcept {
        if (!hasValidOffsets()) {
            return false;
        }
        return std::fabs(centerOffsetMeters()) <= tolerance;
    }

    std::ostream& operator<<(std::ostream& os, const LaneState& state) {
        os << "LaneState{"
           << " left_type=" << static_cast<int>(state.laneTypeLeft())
           << ", right_type=" << static_cast<int>(state.laneTypeRight())
           << ", left_offset_m=" << state.leftOffsetMeters()
           << ", right_offset_m=" << state.rightOffsetMeters()
           << ", allowed_manoeuvres=0x" << std::hex
           << static_cast<int>(state.rawAllowedManeuvers()) << std::dec
           << ", quality=" << static_cast<int>(state.qualityRaw())
           << ", timestamp_ms=" << state.timestampMs()
           << ", seq=" << static_cast<int>(state.seq())
           << ", valid=" << std::boolalpha << state.isValid() << std::noboolalpha
           << " }";
        return os;
    }
}