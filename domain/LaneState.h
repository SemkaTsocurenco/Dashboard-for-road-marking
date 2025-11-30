#pragma once

#include <cstdint>
#include "proto_parser.h"
#include <iosfwd>
#include <array>


namespace domain {
    class LaneState {
    private:
        laneproto::LaneType lane_type_left_{};
        laneproto::LaneType lane_type_right_{};
        float left_offset_m_ = 0.0f;
        float right_offset_m_ = 0.0f;
        std::uint8_t allowed_maneuvers_raw_ = 0;
        std::uint8_t quality_raw_ = 0;
        laneproto::TimestampMs timestamp_ms_{};
        laneproto::SequenceNumber seq_{};
        bool valid_ = false;
        bool has_details_ = false;

        laneproto::LineColor lane_color_left_{laneproto::LineColor::Unknown};
        laneproto::LineColor lane_color_right_{laneproto::LineColor::Unknown};
        float lane_width_left_m_{0.0f};
        float lane_width_right_m_{0.0f};
        std::uint8_t lane_quality_left_{0};
        std::uint8_t lane_quality_right_{0};
        std::array<laneproto::LanePoint, 3> left_points_{};
        std::array<laneproto::LanePoint, 3> right_points_{};
        std::uint8_t left_points_count_{0};
        std::uint8_t right_points_count_{0};

    public:
        LaneState() noexcept = default;
        ~LaneState() noexcept = default;

        LaneState(const LaneState&) noexcept = default;
        LaneState& operator=(const LaneState&) noexcept = default;
        LaneState(LaneState&&) noexcept = default;
        LaneState& operator=(LaneState&&) noexcept = default;

        void updateFromProto(const laneproto::LaneSummary& msg) noexcept;
        void updateFromProto(const laneproto::LaneDetails& msg) noexcept;
        void reset() noexcept;

        laneproto::LaneType laneTypeLeft() const noexcept;
        laneproto::LaneType laneTypeRight() const noexcept;
        laneproto::LineColor laneColorLeft() const noexcept { return lane_color_left_; }
        laneproto::LineColor laneColorRight() const noexcept { return lane_color_right_; }
        float laneWidthLeftMeters() const noexcept { return lane_width_left_m_; }
        float laneWidthRightMeters() const noexcept { return lane_width_right_m_; }
        std::uint8_t laneQualityLeftRaw() const noexcept { return lane_quality_left_; }
        std::uint8_t laneQualityRightRaw() const noexcept { return lane_quality_right_; }
        std::uint8_t leftPointsCount() const noexcept { return left_points_count_; }
        std::uint8_t rightPointsCount() const noexcept { return right_points_count_; }
        const std::array<laneproto::LanePoint, 3>& leftPoints() const noexcept { return left_points_; }
        const std::array<laneproto::LanePoint, 3>& rightPoints() const noexcept { return right_points_; }

        float leftOffsetMeters() const noexcept;
        float rightOffsetMeters() const noexcept;

        std::uint8_t rawAllowedManeuvers() const noexcept;
        std::uint8_t qualityRaw() const noexcept;
        laneproto::TimestampMs timestampMs() const noexcept;
        laneproto::SequenceNumber seq() const noexcept;
        bool isValid() const noexcept;
        bool hasDetails() const noexcept { return has_details_; }

        bool isQualityGood(std::uint8_t threshold) const noexcept;
        float laneWidthMeters() const noexcept;
        float centerOffsetMeters() const noexcept;

        bool isManeuverAllowed(std::uint8_t maneuver_bit) const noexcept;
        bool hasValidOffsets() const noexcept;
        bool isSymmetric(float tolerance = 0.1f) const noexcept;
    };

    std::ostream& operator<<(std::ostream& os, const LaneState& state);
}
