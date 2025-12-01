#include "LaneState.h"
#include <ostream>
#include <cmath>
#include <algorithm>

namespace domain {

    namespace {
        void copyPoints(const std::array<laneproto::LanePoint, 3>& src,
                        std::uint8_t count,
                        std::array<laneproto::LanePoint, 3>& dst,
                        std::uint8_t& dst_count) {
            dst_count = std::min<std::uint8_t>(count, static_cast<std::uint8_t>(dst.size()));
            for (std::size_t i = 0; i < dst.size(); ++i) {
                if (i < dst_count) {
                    dst[i] = src[i];
                } else {
                    dst[i] = {};
                }
            }
        }
    }

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

        // Reset details to avoid showing stale data until a new LaneDetails arrives
        has_details_ = false;
        lane_color_left_ = laneproto::LineColor::Unknown;
        lane_color_right_ = laneproto::LineColor::Unknown;
        lane_width_left_m_ = 0.0f;
        lane_width_right_m_ = 0.0f;
        lane_quality_left_ = quality_raw_;
        lane_quality_right_ = quality_raw_;
        left_points_count_ = 0;
        right_points_count_ = 0;
        left_points_.fill({});
        right_points_.fill({});
    }

    void LaneState::updateFromProto(const laneproto::LaneDetails& msg) noexcept {
        timestamp_ms_ = msg.timestamp_ms;
        seq_ = msg.seq;
        lane_type_left_ = msg.left.type;
        lane_type_right_ = msg.right.type;
        valid_ = true;

        lane_color_left_ = msg.left.color;
        lane_color_right_ = msg.right.color;
        lane_width_left_m_ = msg.left.width_m;
        lane_width_right_m_ = msg.right.width_m;
        lane_quality_left_ = msg.left.quality;
        lane_quality_right_ = msg.right.quality;

        copyPoints(msg.left.points, msg.left.points_count, left_points_, left_points_count_);
        copyPoints(msg.right.points, msg.right.points_count, right_points_, right_points_count_);

        has_details_ = true;
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
        has_details_ = false;
        lane_color_left_ = laneproto::LineColor::Unknown;
        lane_color_right_ = laneproto::LineColor::Unknown;
        lane_width_left_m_ = 0.0f;
        lane_width_right_m_ = 0.0f;
        lane_quality_left_ = 0;
        lane_quality_right_ = 0;
        left_points_count_ = 0;
        right_points_count_ = 0;
        left_points_.fill({});
        right_points_.fill({});
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
           << ", has_details=" << std::boolalpha << state.hasDetails() << std::noboolalpha
           << ", timestamp_ms=" << state.timestampMs()
           << ", seq=" << static_cast<int>(state.seq())
           << ", valid=" << std::boolalpha << state.isValid() << std::noboolalpha
           << " }";
        return os;
    }
}
