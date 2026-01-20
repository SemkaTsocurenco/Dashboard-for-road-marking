#include "FittedLine.h"
#include <ostream>
#include <stdexcept>
#include <cmath>
#include <algorithm>

namespace domain {

    void FittedLine::updateFromProto(const laneproto::FittedLine& msg) noexcept {
        class_id_ = msg.class_id;
        side_ = msg.side;
        color_ = msg.color;
        style_ = msg.style;
        poly_a_ = msg.poly_a;
        poly_b_ = msg.poly_b;
        poly_c_ = msg.poly_c;
        y_start_ = msg.y_start;
        y_end_ = msg.y_end;
        confidence_ = msg.confidence;
        quality_ = msg.quality;
    }

    void FittedLine::updateFromProtoV2(const laneproto::LaneLine& line) noexcept {
        // V2 protocol doesn't have class_id for lines, set to Unknown
        class_id_ = laneproto::MarkingClassId::Unknown;
        side_ = line.side;
        color_ = line.color;
        style_ = line.style;
        poly_a_ = line.poly_a;
        poly_b_ = line.poly_b;
        poly_c_ = line.poly_c;

        // V2: center and points in meters
        center_x_m_ = line.center_x_m;
        center_y_m_ = line.center_y_m;
        points_m_ = line.points_m;
        points_px_ = line.points_px;

        // Calculate y_start and y_end from points in meters
        // Use the Y coordinates of the first and last points
        if (line.points_m[0].y_m > 0 || line.points_m[2].y_m > 0) {
            // Convert meters to decimeters for compatibility with V1 format
            y_start_ = static_cast<std::int16_t>(line.points_m[0].y_m * 10.0f);
            y_end_ = static_cast<std::int16_t>(line.points_m[2].y_m * 10.0f);
        }

        // V2 doesn't have confidence/quality per line, use defaults
        confidence_ = 255;
        quality_ = 255;
    }

    laneproto::MarkingClassId FittedLine::classId() const noexcept {
        return class_id_;
    }

    laneproto::LineSide FittedLine::side() const noexcept {
        return side_;
    }

    laneproto::LineColor FittedLine::color() const noexcept {
        return color_;
    }

    laneproto::LineStyle FittedLine::style() const noexcept {
        return style_;
    }

    float FittedLine::polyA() const noexcept {
        return poly_a_;
    }

    float FittedLine::polyB() const noexcept {
        return poly_b_;
    }

    float FittedLine::polyC() const noexcept {
        return poly_c_;
    }

    std::int16_t FittedLine::yStart() const noexcept {
        return y_start_;
    }

    std::int16_t FittedLine::yEnd() const noexcept {
        return y_end_;
    }

    std::uint8_t FittedLine::confidence() const noexcept {
        return confidence_;
    }

    std::uint8_t FittedLine::quality() const noexcept {
        return quality_;
    }

    bool FittedLine::isValid() const noexcept {
        // RELAXED VALIDATION - only check for finite coefficients, allow y_end_ <= y_start_
        return std::isfinite(poly_a_) && std::isfinite(poly_b_) && std::isfinite(poly_c_);
        // Original check: && y_end_ > y_start_;
    }

    bool FittedLine::isConfident(std::uint8_t threshold) const noexcept {
        return confidence_ >= threshold;
    }

    std::vector<std::pair<float, float>> FittedLine::generatePoints(int numPoints) const {
        std::vector<std::pair<float, float>> points;

        // REMOVED VALIDATION CHECK - generate points for all lines from TCP
        // if (!isValid() || numPoints < 2) {
        //     return points;
        // }

        if (numPoints < 2) {
            numPoints = 2; // Ensure at least 2 points
        }

        // V2 Protocol: Use pixel points directly if available
        // Check if we have valid pixel points from V2 protocol
        bool hasV2PixelPoints = false;
        for (const auto& pt : points_px_) {
            if (pt.x_px != 0.0f || pt.y_px != 0.0f) {
                hasV2PixelPoints = true;
                break;
            }
        }

        if (hasV2PixelPoints) {
            // V2 Protocol: Use the 3 pixel points sent from detector
            // These are already in the correct pixel coordinate space
            // Interpolate between them to generate numPoints

            // First, collect valid points and sort by Y (top to bottom, smallest Y first)
            std::vector<std::pair<float, float>> v2Points;
            for (const auto& pt : points_px_) {
                if (std::isfinite(pt.x_px) && std::isfinite(pt.y_px)) {
                    v2Points.emplace_back(pt.x_px, pt.y_px);
                }
            }

            // REMOVED SIZE CHECK - try to draw even with less points
            // if (v2Points.size() < 2) {
            //     return points;
            // }

            if (v2Points.size() < 1) {
                return points; // Can't draw without any points
            }

            // Sort by Y coordinate
            std::sort(v2Points.begin(), v2Points.end(),
                [](const auto& a, const auto& b) { return a.second < b.second; });

            // Use polynomial with correct pixel-space Y range from V2 points
            const float y_start_px = v2Points.front().second;
            const float y_end_px = v2Points.back().second;
            const float y_range = y_end_px - y_start_px;

            // REMOVED Y_RANGE CHECK - draw even very short lines
            // if (std::abs(y_range) < 1.0f) {
            //     // Very short line, just return the two endpoints
            //     points = v2Points;
            //     return points;
            // }

            // Handle edge case: if y_range is zero or very small, just return the raw points
            if (std::abs(y_range) < 0.001f) {
                points = v2Points;
                return points;
            }

            points.reserve(numPoints);
            const float step = y_range / static_cast<float>(numPoints - 1);

            for (int i = 0; i < numPoints; ++i) {
                const float y = y_start_px + step * static_cast<float>(i);
                // Polynomial: x = a * y^2 + b * y + c (using pixel Y values)
                const float x = poly_a_ * y * y + poly_b_ * y + poly_c_;

                if (std::isfinite(x) && std::isfinite(y)) {
                    points.emplace_back(x, y);
                }
            }

            return points;
        }

        // V1 Protocol fallback: Use y_start_ and y_end_ (in decimeters/pixels)
        points.reserve(numPoints);

        const float y_range = static_cast<float>(y_end_ - y_start_);
        const float step = y_range / static_cast<float>(numPoints - 1);

        for (int i = 0; i < numPoints; ++i) {
            const float y = static_cast<float>(y_start_) + step * static_cast<float>(i);
            // Polynomial: x = a * y^2 + b * y + c
            const float x = poly_a_ * y * y + poly_b_ * y + poly_c_;

            if (std::isfinite(x) && std::isfinite(y)) {
                points.emplace_back(x, y);
            }
        }

        return points;
    }

    float FittedLine::getStartDistance() const noexcept {
        if (!isValid()) {
            return 0.0f;
        }
        const float y = static_cast<float>(y_start_);
        const float x = poly_a_ * y * y + poly_b_ * y + poly_c_;
        // Note: y_start_ and y_end_ are in pixels, need to convert to meters
        // According to worldToImage: y_dm = ((h - cy) / h) * 100
        // Distance from camera (meters): sqrt(x^2 + y^2) / 10 (convert decimeters to meters)
        return std::sqrt(x * x + y * y) / 10.0f;
    }

    float FittedLine::getMiddleDistance() const noexcept {
        if (!isValid()) {
            return 0.0f;
        }
        const float y = static_cast<float>(y_start_ + y_end_) / 2.0f;
        const float x = poly_a_ * y * y + poly_b_ * y + poly_c_;
        return std::sqrt(x * x + y * y) / 10.0f;
    }

    float FittedLine::getEndDistance() const noexcept {
        if (!isValid()) {
            return 0.0f;
        }
        const float y = static_cast<float>(y_end_);
        const float x = poly_a_ * y * y + poly_b_ * y + poly_c_;
        return std::sqrt(x * x + y * y) / 10.0f;
    }

    std::ostream& operator<<(std::ostream& os, const FittedLine& line) {
        os << "FittedLine{"
           << " class_id=" << static_cast<int>(line.classId())
           << ", side=" << static_cast<int>(line.side())
           << ", color=" << static_cast<int>(line.color())
           << ", style=" << static_cast<int>(line.style())
           << ", poly=[" << line.polyA() << "," << line.polyB() << "," << line.polyC() << "]"
           << ", y_range=[" << line.yStart() << "," << line.yEnd() << "]"
           << ", confidence=" << static_cast<int>(line.confidence())
           << ", quality=" << static_cast<int>(line.quality())
           << " }";
        return os;
    }

    void FittedLinesModel::updateFromProto(const laneproto::FittedLines& msg) {
        timestamp_ms_ = msg.timestamp_ms;
        seq_ = msg.seq;

        lines_.clear();
        lines_.reserve(msg.lines.size());

        for (const auto& line_proto : msg.lines) {
            FittedLine line;
            line.updateFromProto(line_proto);
            lines_.push_back(line);
        }

        valid_ = true;
    }

    void FittedLinesModel::updateFromProtoV2(const laneproto::LaneLines& msg) {
        timestamp_ms_ = msg.timestamp_ms;
        seq_ = msg.seq;

        lines_.clear();
        lines_.reserve(msg.lines.size());

        for (const auto& line_proto : msg.lines) {
            FittedLine line;
            line.updateFromProtoV2(line_proto);
            lines_.push_back(line);
        }

        valid_ = true;
    }

    std::size_t FittedLinesModel::size() const noexcept {
        return lines_.size();
    }

    bool FittedLinesModel::empty() const noexcept {
        return lines_.empty();
    }

    bool FittedLinesModel::isValid() const noexcept {
        return valid_;
    }

    const FittedLine& FittedLinesModel::at(std::size_t index) const {
        if (index >= lines_.size()) {
            throw std::out_of_range("FittedLinesModel::at: index out of range");
        }
        return lines_[index];
    }

    const FittedLine& FittedLinesModel::operator[](std::size_t index) const noexcept {
        return lines_[index];
    }

    const std::vector<FittedLine>& FittedLinesModel::lines() const noexcept {
        return lines_;
    }

    laneproto::TimestampMs FittedLinesModel::timestampMs() const noexcept {
        return timestamp_ms_;
    }

    laneproto::SequenceNumber FittedLinesModel::seq() const noexcept {
        return seq_;
    }

    void FittedLinesModel::clear() noexcept {
        lines_.clear();
        timestamp_ms_ = {};
        seq_ = {};
        valid_ = false;
    }

    void FittedLinesModel::reserve(std::size_t capacity) {
        lines_.reserve(capacity);
    }

    std::ostream& operator<<(std::ostream& os, const FittedLinesModel& model) {
        os << "FittedLinesModel{"
           << " timestamp_ms=" << model.timestampMs()
           << ", seq=" << static_cast<int>(model.seq())
           << ", lines_count=" << model.size()
           << ", valid=" << std::boolalpha << model.isValid() << std::noboolalpha
           << " }";
        return os;
    }

} // namespace domain
