#include "Warning.h"
#include <ostream>
#include <stdexcept>
#include <algorithm>

namespace domain {

    Warning::Warning(WarningType type, WarningSeverity severity, std::uint64_t timestamp_ms,
                     float distance_m, std::uint8_t confidence)
        : type_(type)
        , severity_(severity)
        , timestamp_ms_(timestamp_ms)
        , distance_m_(distance_m)
        , confidence_(confidence)
        , active_(true)
    {}

    WarningType Warning::type() const noexcept {
        return type_;
    }

    WarningSeverity Warning::severity() const noexcept {
        return severity_;
    }

    std::uint64_t Warning::timestampMs() const noexcept {
        return timestamp_ms_;
    }

    float Warning::distanceMeters() const noexcept {
        return distance_m_;
    }

    std::uint8_t Warning::confidence() const noexcept {
        return confidence_;
    }

    const std::string& Warning::message() const noexcept {
        return message_;
    }

    bool Warning::isActive() const noexcept {
        return active_;
    }

    void Warning::setType(WarningType type) noexcept {
        type_ = type;
    }

    void Warning::setSeverity(WarningSeverity severity) noexcept {
        severity_ = severity;
    }

    void Warning::setTimestampMs(std::uint64_t timestamp_ms) noexcept {
        timestamp_ms_ = timestamp_ms;
    }

    void Warning::setDistanceMeters(float distance_m) noexcept {
        distance_m_ = distance_m;
    }

    void Warning::setConfidence(std::uint8_t confidence) noexcept {
        confidence_ = confidence;
    }

    void Warning::setMessage(const std::string& message) {
        message_ = message;
    }

    void Warning::setMessage(std::string&& message) noexcept {
        message_ = std::move(message);
    }

    void Warning::setActive(bool active) noexcept {
        active_ = active;
    }

    bool Warning::isCritical() const noexcept {
        return severity_ == WarningSeverity::Critical;
    }

    bool Warning::isConfident(std::uint8_t threshold) const noexcept {
        return confidence_ >= threshold;
    }

    bool Warning::isValid() const noexcept {
        return type_ != WarningType::Unknown;
    }

    void Warning::deactivate() noexcept {
        active_ = false;
    }

    void Warning::reset() noexcept {
        type_ = WarningType::Unknown;
        severity_ = WarningSeverity::Info;
        timestamp_ms_ = 0;
        distance_m_ = 0.0f;
        confidence_ = 0;
        message_.clear();
        active_ = false;
    }

    std::ostream& operator<<(std::ostream& os, WarningType type) {
        switch (type) {
            case WarningType::Unknown:
                return os << "Unknown";
            case WarningType::CrosswalkAhead:
                return os << "CrosswalkAhead";
            case WarningType::LaneDepartureLeft:
                return os << "LaneDepartureLeft";
            case WarningType::LaneDepartureRight:
                return os << "LaneDepartureRight";
            case WarningType::SolidLineCross:
                return os << "SolidLineCross";
            case WarningType::Custom:
                return os << "Custom";
            default:
                return os << "Unknown(" << static_cast<int>(type) << ")";
        }
    }

    std::ostream& operator<<(std::ostream& os, WarningSeverity severity) {
        switch (severity) {
            case WarningSeverity::Info:
                return os << "Info";
            case WarningSeverity::Warning:
                return os << "Warning";
            case WarningSeverity::Critical:
                return os << "Critical";
            default:
                return os << "Unknown(" << static_cast<int>(severity) << ")";
        }
    }

    std::ostream& operator<<(std::ostream& os, const Warning& warning) {
        os << "Warning{"
           << " type=" << warning.type()
           << ", severity=" << warning.severity()
           << ", timestamp_ms=" << warning.timestampMs()
           << ", distance_m=" << warning.distanceMeters()
           << ", confidence=" << static_cast<int>(warning.confidence())
           << ", active=" << std::boolalpha << warning.isActive() << std::noboolalpha;

        if (!warning.message().empty()) {
            os << ", message=\"" << warning.message() << "\"";
        }

        os << " }";
        return os;
    }

    void WarningModel::addWarning(const Warning& warning) {
        warnings_.push_back(warning);
        valid_ = true;
    }

    void WarningModel::addWarning(Warning&& warning) {
        warnings_.push_back(std::move(warning));
        valid_ = true;
    }

    std::size_t WarningModel::size() const noexcept {
        return warnings_.size();
    }

    bool WarningModel::empty() const noexcept {
        return warnings_.empty();
    }

    bool WarningModel::isValid() const noexcept {
        return valid_;
    }

    const Warning& WarningModel::at(std::size_t index) const {
        if (index >= warnings_.size()) {
            throw std::out_of_range("WarningModel::at: index out of range");
        }
        return warnings_[index];
    }

    const Warning& WarningModel::operator[](std::size_t index) const noexcept {
        return warnings_[index];
    }

    const std::vector<Warning>& WarningModel::warnings() const noexcept {
        return warnings_;
    }

    std::uint64_t WarningModel::lastUpdateMs() const noexcept {
        return last_update_ms_;
    }

    void WarningModel::setLastUpdateMs(std::uint64_t timestamp_ms) noexcept {
        last_update_ms_ = timestamp_ms;
    }

    void WarningModel::clear() noexcept {
        warnings_.clear();
        last_update_ms_ = 0;
        valid_ = false;
    }

    void WarningModel::reserve(std::size_t capacity) {
        warnings_.reserve(capacity);
    }

    void WarningModel::removeInactive() {
        warnings_.erase(
            std::remove_if(warnings_.begin(), warnings_.end(),
                [](const Warning& w) { return !w.isActive(); }),
            warnings_.end()
        );
    }

    std::size_t WarningModel::countActive() const noexcept {
        return std::count_if(warnings_.begin(), warnings_.end(),
            [](const Warning& w) { return w.isActive(); });
    }

    std::size_t WarningModel::countBySeverity(WarningSeverity severity) const noexcept {
        return std::count_if(warnings_.begin(), warnings_.end(),
            [severity](const Warning& w) { return w.severity() == severity && w.isActive(); });
    }

    std::size_t WarningModel::countByType(WarningType type) const noexcept {
        return std::count_if(warnings_.begin(), warnings_.end(),
            [type](const Warning& w) { return w.type() == type && w.isActive(); });
    }

    bool WarningModel::hasCriticalWarnings() const noexcept {
        return std::any_of(warnings_.begin(), warnings_.end(),
            [](const Warning& w) { return w.isCritical() && w.isActive(); });
    }

    std::ostream& operator<<(std::ostream& os, const WarningModel& model) {
        os << "WarningModel{"
           << " count=" << model.size()
           << ", active=" << model.countActive()
           << ", critical=" << model.countBySeverity(WarningSeverity::Critical)
           << ", last_update_ms=" << model.lastUpdateMs()
           << ", valid=" << std::boolalpha << model.isValid() << std::noboolalpha
           << " }";
        return os;
    }
}
