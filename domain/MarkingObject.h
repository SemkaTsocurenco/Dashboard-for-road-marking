#pragma once

#include "proto_parser.h"
#include <cstdint>
#include <vector>
#include <iosfwd>

namespace domain {
    class MarkingObject {
    private:
        laneproto::MarkingClassId class_id_{};
        float x_m_ = 0.0f;
        float y_m_ = 0.0f;
        float length_m_ = 0.0f;
        float width_m_ = 0.0f;
        float yaw_deg_ = 0.0f;
        std::uint8_t confidence_ = 0;
        std::uint8_t flags_ = 0;

    public:
        MarkingObject() noexcept = default;
        ~MarkingObject() noexcept = default;

        void updateFromProto(const laneproto::MarkingObject& msg) noexcept;

        laneproto::MarkingClassId classId() const noexcept;
        float xMeters() const noexcept;
        float yMeters() const noexcept;
        float lengthMeters() const noexcept;
        float widthMeters() const noexcept;
        float yawDeg() const noexcept;
        std::uint8_t confidence() const noexcept;
        std::uint8_t rawFlags() const noexcept;

        bool isCrosswalk() const noexcept;
        bool isArrow() const noexcept;
        bool hasFlag(std::uint8_t mask) const noexcept;
        bool isConfident(std::uint8_t threshold = 50) const noexcept;
        bool isValid() const noexcept;
        float area() const noexcept;
    };

    std::ostream& operator<<(std::ostream& os, const MarkingObject& obj);

    class MarkingObjectModel {
    private:
        laneproto::TimestampMs timestamp_ms_{};
        laneproto::SequenceNumber seq_{};
        std::vector<MarkingObject> objects_;
        bool valid_ = false;

    public:
        MarkingObjectModel() noexcept = default;
        ~MarkingObjectModel() noexcept = default;
        
        void updateFromProto(const laneproto::MarkingObjects& msg);

        std::size_t size() const noexcept;
        bool empty() const noexcept;
        bool isValid() const noexcept;

        const MarkingObject& at(std::size_t index) const;
        const MarkingObject& operator[](std::size_t index) const noexcept;
        const std::vector<MarkingObject>& objects() const noexcept;

        laneproto::TimestampMs timestampMs() const noexcept;
        laneproto::SequenceNumber seq() const noexcept;

        void clear() noexcept;
        void reserve(std::size_t capacity);

        auto begin() const noexcept { return objects_.begin(); }
        auto end() const noexcept { return objects_.end(); }
        auto cbegin() const noexcept { return objects_.cbegin(); }
        auto cend() const noexcept { return objects_.cend(); }
    };

    std::ostream& operator<<(std::ostream& os, const MarkingObjectModel& model);
}
