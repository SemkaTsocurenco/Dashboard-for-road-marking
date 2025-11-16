#pragma once

#include <cstdint>
#include "proto_parser.h"
#include <iosfwd>


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

    public:
        LaneState() noexcept = default;
        ~LaneState() noexcept = default;

        LaneState(const LaneState&) noexcept = default;
        LaneState& operator=(const LaneState&) noexcept = default;
        LaneState(LaneState&&) noexcept = default;
        LaneState& operator=(LaneState&&) noexcept = default;

        void updateFromProto(const laneproto::LaneSummary& msg) noexcept;
        void reset() noexcept;

        laneproto::LaneType laneTypeLeft() const noexcept;
        laneproto::LaneType laneTypeRight() const noexcept;

        float leftOffsetMeters() const noexcept;
        float rightOffsetMeters() const noexcept;

        std::uint8_t rawAllowedManeuvers() const noexcept;
        std::uint8_t qualityRaw() const noexcept;
        laneproto::TimestampMs timestampMs() const noexcept;
        laneproto::SequenceNumber seq() const noexcept;
        bool isValid() const noexcept;

        bool isQualityGood(std::uint8_t threshold) const noexcept;
        float laneWidthMeters() const noexcept;
        float centerOffsetMeters() const noexcept;

        bool isManeuverAllowed(std::uint8_t maneuver_bit) const noexcept;
        bool hasValidOffsets() const noexcept;
        bool isSymmetric(float tolerance = 0.1f) const noexcept;
    };

    std::ostream& operator<<(std::ostream& os, const LaneState& state);
}