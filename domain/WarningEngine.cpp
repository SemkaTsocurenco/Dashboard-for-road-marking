#include "WarningEngine.h"
#include <cmath>
#include <string>

namespace domain {

    WarningEngine::WarningEngine(const WarningEngineConfig& config) noexcept
        : config_(config)
    {}

    const WarningEngineConfig& WarningEngine::config() const noexcept {
        return config_;
    }

    void WarningEngine::setConfig(const WarningEngineConfig& config) noexcept {
        config_ = config;
    }

    Warning WarningEngine::makeCrosswalkWarning(const MarkingObject& obj,
                                                std::uint64_t timestamp_ms) const {
        float distance = obj.xMeters();
        WarningSeverity severity = WarningSeverity::Warning;

        if (distance < config_.crosswalk_critical_distance_m) {
            severity = WarningSeverity::Critical;
        }

        Warning w{
            WarningType::CrosswalkAhead,
            severity,
            timestamp_ms,
            distance,
            obj.confidence()
        };

        std::string msg = "Crosswalk ahead at ";
        msg += std::to_string(static_cast<int>(distance));
        msg += " m";
        w.setMessage(std::move(msg));

        return w;
    }

    void WarningEngine::addCrosswalkWarnings(const MarkingObjectModel& markings,
                                             std::uint64_t timestamp_ms,
                                             std::vector<Warning>& out) const {
        if (!config_.enable_crosswalk_warnings) {
            return;
        }

        for (const auto& obj : markings) {
            if (!obj.isCrosswalk()) {
                continue;
            }

            if (!obj.isConfident(config_.min_marking_confidence)) {
                continue;
            }

            float distance = obj.xMeters();
            if (distance < 0.0f || distance > config_.crosswalk_distance_threshold_m) {
                continue;
            }

            out.push_back(makeCrosswalkWarning(obj, timestamp_ms));
        }
    }

    void WarningEngine::addLaneDepartureWarnings(const LaneState& lane,
                                                  std::uint64_t timestamp_ms,
                                                  std::vector<Warning>& out) const {
        if (!config_.enable_lane_departure_warnings) {
            return;
        }

        if (!lane.isValid()) {
            return;
        }

        if (!lane.isQualityGood(config_.min_lane_quality)) {
            return;
        }

        if (!lane.hasValidOffsets()) {
            return;
        }

        float center_offset = lane.centerOffsetMeters();

        if (center_offset < -config_.lane_departure_offset_threshold_m) {
            Warning w{
                WarningType::LaneDepartureLeft,
                WarningSeverity::Warning,
                timestamp_ms,
                std::fabs(center_offset),
                lane.qualityRaw()
            };
            std::string msg = "Lane departure left: offset ";
            msg += std::to_string(static_cast<int>(std::fabs(center_offset) * 100));
            msg += " cm";
            w.setMessage(std::move(msg));
            out.push_back(std::move(w));
        } else if (center_offset > config_.lane_departure_offset_threshold_m) {
            Warning w{
                WarningType::LaneDepartureRight,
                WarningSeverity::Warning,
                timestamp_ms,
                std::fabs(center_offset),
                lane.qualityRaw()
            };
            std::string msg = "Lane departure right: offset ";
            msg += std::to_string(static_cast<int>(std::fabs(center_offset) * 100));
            msg += " cm";
            w.setMessage(std::move(msg));
            out.push_back(std::move(w));
        }
    }

    std::vector<Warning> WarningEngine::update(const LaneState& lane,
                                               const MarkingObjectModel& markings,
                                               std::uint64_t timestamp_ms) const {
        std::vector<Warning> result;
        result.reserve(8);

        addCrosswalkWarnings(markings, timestamp_ms, result);
        addLaneDepartureWarnings(lane, timestamp_ms, result);

        return result;
    }

}