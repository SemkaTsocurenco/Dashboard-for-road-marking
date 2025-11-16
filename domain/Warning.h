#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <iosfwd>

namespace domain {

    enum class WarningType : std::uint8_t {
        Unknown = 0,
        CrosswalkAhead,
        LaneDepartureLeft,
        LaneDepartureRight,
        SolidLineCross,
        Custom
    };

    enum class WarningSeverity : std::uint8_t {
        Info = 0,
        Warning,
        Critical
    };

    class Warning {
    private:
        WarningType type_ = WarningType::Unknown;
        WarningSeverity severity_ = WarningSeverity::Info;
        std::uint64_t timestamp_ms_ = 0;
        float distance_m_ = 0.0f;
        std::uint8_t confidence_ = 0;
        std::string message_;
        bool active_ = false;

    public:
        Warning() noexcept = default;
        ~Warning() noexcept = default;

        Warning(WarningType type, WarningSeverity severity, std::uint64_t timestamp_ms,
                float distance_m = 0.0f, std::uint8_t confidence = 100);

        WarningType type() const noexcept;
        WarningSeverity severity() const noexcept;
        std::uint64_t timestampMs() const noexcept;
        float distanceMeters() const noexcept;
        std::uint8_t confidence() const noexcept;
        const std::string& message() const noexcept;
        bool isActive() const noexcept;

        void setType(WarningType type) noexcept;
        void setSeverity(WarningSeverity severity) noexcept;
        void setTimestampMs(std::uint64_t timestamp_ms) noexcept;
        void setDistanceMeters(float distance_m) noexcept;
        void setConfidence(std::uint8_t confidence) noexcept;
        void setMessage(const std::string& message);
        void setMessage(std::string&& message) noexcept;
        void setActive(bool active) noexcept;

        bool isCritical() const noexcept;
        bool isConfident(std::uint8_t threshold = 50) const noexcept;
        bool isValid() const noexcept;

        void deactivate() noexcept;
        void reset() noexcept;
    };

    std::ostream& operator<<(std::ostream& os, WarningType type);
    std::ostream& operator<<(std::ostream& os, WarningSeverity severity);
    std::ostream& operator<<(std::ostream& os, const Warning& warning);

    class WarningModel {
    private:
        std::vector<Warning> warnings_;
        std::uint64_t last_update_ms_ = 0;
        bool valid_ = false;

    public:
        WarningModel() noexcept = default;
        ~WarningModel() noexcept = default;

        WarningModel(const WarningModel&) = default;
        WarningModel& operator=(const WarningModel&) = default;
        WarningModel(WarningModel&&) noexcept = default;
        WarningModel& operator=(WarningModel&&) noexcept = default;

        void addWarning(const Warning& warning);
        void addWarning(Warning&& warning);

        std::size_t size() const noexcept;
        bool empty() const noexcept;
        bool isValid() const noexcept;

        const Warning& at(std::size_t index) const;
        const Warning& operator[](std::size_t index) const noexcept;
        const std::vector<Warning>& warnings() const noexcept;

        std::uint64_t lastUpdateMs() const noexcept;
        void setLastUpdateMs(std::uint64_t timestamp_ms) noexcept;

        void clear() noexcept;
        void reserve(std::size_t capacity);
        void removeInactive();

        std::size_t countActive() const noexcept;
        std::size_t countBySeverity(WarningSeverity severity) const noexcept;
        std::size_t countByType(WarningType type) const noexcept;
        bool hasCriticalWarnings() const noexcept;

        auto begin() const noexcept { return warnings_.begin(); }
        auto end() const noexcept { return warnings_.end(); }
        auto cbegin() const noexcept { return warnings_.cbegin(); }
        auto cend() const noexcept { return warnings_.cend(); }
    };

    std::ostream& operator<<(std::ostream& os, const WarningModel& model);
}
