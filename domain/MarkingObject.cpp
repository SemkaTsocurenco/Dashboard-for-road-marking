#include "MarkingObject.h"
#include <ostream>
#include <stdexcept>
#include <cmath>

namespace domain {

    void MarkingObject::updateFromProto(const laneproto::MarkingObject& msg) noexcept {
        class_id_ = msg.class_id;
        x_m_ = msg.x_m;
        y_m_ = msg.y_m;
        length_m_ = msg.length_m;
        width_m_ = msg.width_m;
        yaw_deg_ = msg.yaw_deg;
        confidence_ = msg.confidence;
        flags_ = msg.flags;
    }

    laneproto::MarkingClassId MarkingObject::classId() const noexcept {
        return class_id_;
    }

    float MarkingObject::xMeters() const noexcept {
        return x_m_;
    }

    float MarkingObject::yMeters() const noexcept {
        return y_m_;
    }

    float MarkingObject::lengthMeters() const noexcept {
        return length_m_;
    }

    float MarkingObject::widthMeters() const noexcept {
        return width_m_;
    }

    float MarkingObject::yawDeg() const noexcept {
        return yaw_deg_;
    }

    std::uint8_t MarkingObject::confidence() const noexcept {
        return confidence_;
    }

    std::uint8_t MarkingObject::rawFlags() const noexcept {
        return flags_;
    }

    bool MarkingObject::isCrosswalk() const noexcept {
        return class_id_ == laneproto::MarkingClassId::Crosswalk;
    }

    bool MarkingObject::isArrow() const noexcept {
        return class_id_ == laneproto::MarkingClassId::Arrow;
    }

    bool MarkingObject::hasFlag(std::uint8_t mask) const noexcept {
        return (flags_ & mask) != 0;
    }

    bool MarkingObject::isConfident(std::uint8_t threshold) const noexcept {
        return confidence_ >= threshold;
    }

    bool MarkingObject::isValid() const noexcept {
        return std::isfinite(x_m_) && std::isfinite(y_m_)
            && std::isfinite(length_m_) && std::isfinite(width_m_)
            && std::isfinite(yaw_deg_)
            && length_m_ > 0.0f && width_m_ > 0.0f;
    }

    float MarkingObject::area() const noexcept {
        return length_m_ * width_m_;
    }

    std::ostream& operator<<(std::ostream& os, const MarkingObject& obj) {
        os << "MarkingObject{"
           << " class_id=" << static_cast<int>(obj.classId())
           << ", x_m=" << obj.xMeters()
           << ", y_m=" << obj.yMeters()
           << ", length_m=" << obj.lengthMeters()
           << ", width_m=" << obj.widthMeters()
           << ", yaw_deg=" << obj.yawDeg()
           << ", confidence=" << static_cast<int>(obj.confidence())
           << ", flags=0x" << std::hex << static_cast<int>(obj.rawFlags()) << std::dec
           << " }";
        return os;
    }

    void MarkingObjectModel::updateFromProto(const laneproto::MarkingObjects& msg) {
        timestamp_ms_ = msg.timestamp_ms;
        seq_ = msg.seq;

        objects_.clear();
        objects_.reserve(msg.objects.size());

        for (const auto& obj_proto : msg.objects) {
            MarkingObject obj;
            obj.updateFromProto(obj_proto);
            objects_.push_back(obj);
        }

        valid_ = true;
    }

    std::size_t MarkingObjectModel::size() const noexcept {
        return objects_.size();
    }

    bool MarkingObjectModel::empty() const noexcept {
        return objects_.empty();
    }

    bool MarkingObjectModel::isValid() const noexcept {
        return valid_;
    }

    const MarkingObject& MarkingObjectModel::at(std::size_t index) const {
        if (index >= objects_.size()) {
            throw std::out_of_range("MarkingObjectModel::at: index out of range");
        }
        return objects_[index];
    }

    const MarkingObject& MarkingObjectModel::operator[](std::size_t index) const noexcept {
        return objects_[index];
    }

    const std::vector<MarkingObject>& MarkingObjectModel::objects() const noexcept {
        return objects_;
    }

    laneproto::TimestampMs MarkingObjectModel::timestampMs() const noexcept {
        return timestamp_ms_;
    }

    laneproto::SequenceNumber MarkingObjectModel::seq() const noexcept {
        return seq_;
    }

    void MarkingObjectModel::clear() noexcept {
        objects_.clear();
        timestamp_ms_ = {};
        seq_ = {};
        valid_ = false;
    }

    void MarkingObjectModel::reserve(std::size_t capacity) {
        objects_.reserve(capacity);
    }

    std::ostream& operator<<(std::ostream& os, const MarkingObjectModel& model) {
        os << "MarkingObjectModel{"
           << " timestamp_ms=" << model.timestampMs()
           << ", seq=" << static_cast<int>(model.seq())
           << ", objects_count=" << model.size()
           << ", valid=" << std::boolalpha << model.isValid() << std::noboolalpha
           << " }";
        return os;
    }
}
