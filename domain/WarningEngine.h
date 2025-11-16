#pragma once

#include "LaneState.h"
#include "MarkingObject.h"
#include "Warning.h"
#include <cstdint>


namespace domain {

    struct WarningEngineConfig {
        float lane_departure_offset_threshold_m = 0.3f;
        float crosswalk_distance_threshold_m = 30.0f;
        float crosswalk_critical_distance_m = 10.0f;
        std::uint8_t min_marking_confidence = 50;
        std::uint8_t min_lane_quality = 60;
        bool enable_crosswalk_warnings = true;
        bool enable_lane_departure_warnings = true;
    };

    class WarningEngine {
    public:
        WarningEngine() = default;
        explicit WarningEngine(const WarningEngineConfig& config) noexcept;

        const WarningEngineConfig& config() const noexcept;
        void setConfig(const WarningEngineConfig& config) noexcept;

        std::vector<Warning> update(const LaneState& lane,
                                    const MarkingObjectModel& markings,
                                    std::uint64_t timestamp_ms) const;

    private:
        WarningEngineConfig config_{};

        void addCrosswalkWarnings(const MarkingObjectModel& markings,
                                  std::uint64_t timestamp_ms,
                                  std::vector<Warning>& out) const;

        void addLaneDepartureWarnings(const LaneState& lane,
                                      std::uint64_t timestamp_ms,
                                      std::vector<Warning>& out) const;

        Warning makeCrosswalkWarning(const MarkingObject& obj,
                                     std::uint64_t timestamp_ms) const;
    };

}