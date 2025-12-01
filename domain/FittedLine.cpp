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
        return std::isfinite(poly_a_) && std::isfinite(poly_b_) && std::isfinite(poly_c_)
            && y_end_ > y_start_;
    }

    bool FittedLine::isConfident(std::uint8_t threshold) const noexcept {
        return confidence_ >= threshold;
    }

    std::vector<std::pair<float, float>> FittedLine::generatePoints(int numPoints) const {
        std::vector<std::pair<float, float>> points;
        if (!isValid() || numPoints < 2) {
            return points;
        }

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
