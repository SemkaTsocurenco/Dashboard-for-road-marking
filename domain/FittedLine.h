#pragma once

#include "proto_parser.h"
#include "AppConfig.hpp"
#include <vector>
#include <array>
#include <ostream>

namespace domain {

    class FittedLine {
    public:
        FittedLine() = default;
        ~FittedLine() = default;

        void updateFromProto(const laneproto::FittedLine& msg) noexcept;
        void updateFromProtoV2(const laneproto::LaneLine& line) noexcept;

        [[nodiscard]] laneproto::MarkingClassId classId() const noexcept;
        [[nodiscard]] laneproto::LineSide side() const noexcept;
        [[nodiscard]] laneproto::LineColor color() const noexcept;
        [[nodiscard]] laneproto::LineStyle style() const noexcept;
        [[nodiscard]] float polyA() const noexcept;
        [[nodiscard]] float polyB() const noexcept;
        [[nodiscard]] float polyC() const noexcept;
        [[nodiscard]] std::int16_t yStart() const noexcept;
        [[nodiscard]] std::int16_t yEnd() const noexcept;
        [[nodiscard]] std::uint8_t confidence() const noexcept;
        [[nodiscard]] std::uint8_t quality() const noexcept;

        [[nodiscard]] laneproto::LineColor lineColor() const noexcept { return color_; }
        [[nodiscard]] laneproto::LineStyle lineStyle() const noexcept { return style_; }

        [[nodiscard]] bool isValid() const noexcept;
        [[nodiscard]] bool isConfident(std::uint8_t threshold = config::ProtocolConfig::QUALITY_RAW_MAX / 2) const noexcept;

        // Generate points along the polynomial curve
        [[nodiscard]] std::vector<std::pair<float, float>> generatePoints(int numPoints = 50) const;

        // Calculate distances (in meters) at key positions
        [[nodiscard]] float getStartDistance() const noexcept;
        [[nodiscard]] float getMiddleDistance() const noexcept;
        [[nodiscard]] float getEndDistance() const noexcept;

        // V2 accessors for points in meters and pixels
        [[nodiscard]] float centerXMeters() const noexcept { return center_x_m_; }
        [[nodiscard]] float centerYMeters() const noexcept { return center_y_m_; }
        [[nodiscard]] const std::array<laneproto::PointMeters, 3>& pointsMeters() const noexcept { return points_m_; }
        [[nodiscard]] const std::array<laneproto::PointPixels, 3>& pointsPixels() const noexcept { return points_px_; }

    private:
        laneproto::MarkingClassId class_id_ = laneproto::MarkingClassId::Unknown;
        laneproto::LineSide side_ = laneproto::LineSide::Unknown;
        laneproto::LineColor color_ = laneproto::LineColor::Unknown;
        laneproto::LineStyle style_ = laneproto::LineStyle::Unknown;
        float poly_a_ = 0.0f;
        float poly_b_ = 0.0f;
        float poly_c_ = 0.0f;
        std::int16_t y_start_ = 0;
        std::int16_t y_end_ = 0;
        std::uint8_t confidence_ = 0;
        std::uint8_t quality_ = 0;

        // V2 fields
        float center_x_m_ = 0.0f;
        float center_y_m_ = 0.0f;
        std::array<laneproto::PointMeters, 3> points_m_{};
        std::array<laneproto::PointPixels, 3> points_px_{};
    };

    std::ostream& operator<<(std::ostream& os, const FittedLine& line);

    class FittedLinesModel {
    public:
        FittedLinesModel() = default;
        ~FittedLinesModel() = default;

        void updateFromProto(const laneproto::FittedLines& msg);
        void updateFromProtoV2(const laneproto::LaneLines& msg);

        [[nodiscard]] std::size_t size() const noexcept;
        [[nodiscard]] bool empty() const noexcept;
        [[nodiscard]] bool isValid() const noexcept;

        [[nodiscard]] const FittedLine& at(std::size_t index) const;
        [[nodiscard]] const FittedLine& operator[](std::size_t index) const noexcept;
        [[nodiscard]] const std::vector<FittedLine>& lines() const noexcept;

        [[nodiscard]] laneproto::TimestampMs timestampMs() const noexcept;
        [[nodiscard]] laneproto::SequenceNumber seq() const noexcept;

        void clear() noexcept;
        void reserve(std::size_t capacity);

    private:
        std::vector<FittedLine> lines_;
        laneproto::TimestampMs timestamp_ms_{};
        laneproto::SequenceNumber seq_{};
        bool valid_ = false;
    };

    std::ostream& operator<<(std::ostream& os, const FittedLinesModel& model);

} // namespace domain
