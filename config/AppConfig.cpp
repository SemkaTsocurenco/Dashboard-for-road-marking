#include "AppConfig.hpp"
#include "WarningEngine.h"

namespace config {


QJsonObject NetworkConfig::toJson() const {
    QJsonObject json;
    json["host"] = host;
    json["port"] = static_cast<int>(port);
    json["reconnect_interval_ms"] = reconnect_interval_ms;
    json["max_reconnect_attempts"] = max_reconnect_attempts;
    json["auto_reconnect"] = auto_reconnect;
    return json;
}

NetworkConfig NetworkConfig::fromJson(const QJsonObject& json) {
    NetworkConfig config;

    if (json.contains("host"))
        config.host = json["host"].toString();

    if (json.contains("port"))
        config.port = static_cast<quint16>(json["port"].toInt());

    if (json.contains("reconnect_interval_ms"))
        config.reconnect_interval_ms = json["reconnect_interval_ms"].toInt();

    if (json.contains("max_reconnect_attempts"))
        config.max_reconnect_attempts = json["max_reconnect_attempts"].toInt();

    if (json.contains("auto_reconnect"))
        config.auto_reconnect = json["auto_reconnect"].toBool();

    return config;
}


QJsonObject VideoConfig::toJson() const {
    QJsonObject json;
    json["source_url"] = source_url;
    json["auto_start"] = auto_start;
    return json;
}

VideoConfig VideoConfig::fromJson(const QJsonObject& json) {
    VideoConfig config;

    if (json.contains("source_url"))
        config.source_url = json["source_url"].toString();

    if (json.contains("auto_start"))
        config.auto_start = json["auto_start"].toBool();

    return config;
}


QJsonObject WarningConfig::toJson() const {
    QJsonObject json;
    json["lane_departure_threshold_m"] = static_cast<double>(lane_departure_threshold_m);
    json["crosswalk_distance_threshold_m"] = static_cast<double>(crosswalk_distance_threshold_m);
    json["crosswalk_critical_distance_m"] = static_cast<double>(crosswalk_critical_distance_m);
    json["min_marking_confidence"] = static_cast<int>(min_marking_confidence);
    json["min_lane_quality"] = static_cast<int>(min_lane_quality);
    json["enable_crosswalk_warnings"] = enable_crosswalk_warnings;
    json["enable_lane_departure_warnings"] = enable_lane_departure_warnings;
    return json;
}

WarningConfig WarningConfig::fromJson(const QJsonObject& json) {
    WarningConfig config;

    if (json.contains("lane_departure_threshold_m"))
        config.lane_departure_threshold_m = static_cast<float>(json["lane_departure_threshold_m"].toDouble());

    if (json.contains("crosswalk_distance_threshold_m"))
        config.crosswalk_distance_threshold_m = static_cast<float>(json["crosswalk_distance_threshold_m"].toDouble());

    if (json.contains("crosswalk_critical_distance_m"))
        config.crosswalk_critical_distance_m = static_cast<float>(json["crosswalk_critical_distance_m"].toDouble());

    if (json.contains("min_marking_confidence"))
        config.min_marking_confidence = static_cast<std::uint8_t>(json["min_marking_confidence"].toInt());

    if (json.contains("min_lane_quality"))
        config.min_lane_quality = static_cast<std::uint8_t>(json["min_lane_quality"].toInt());

    if (json.contains("enable_crosswalk_warnings"))
        config.enable_crosswalk_warnings = json["enable_crosswalk_warnings"].toBool();

    if (json.contains("enable_lane_departure_warnings"))
        config.enable_lane_departure_warnings = json["enable_lane_departure_warnings"].toBool();

    return config;
}

domain::WarningEngineConfig WarningConfig::toDomainConfig() const {
    domain::WarningEngineConfig domain_config;
    domain_config.lane_departure_offset_threshold_m = lane_departure_threshold_m;
    domain_config.crosswalk_distance_threshold_m = crosswalk_distance_threshold_m;
    domain_config.crosswalk_critical_distance_m = crosswalk_critical_distance_m;
    domain_config.min_marking_confidence = min_marking_confidence;
    domain_config.min_lane_quality = min_lane_quality;
    domain_config.enable_crosswalk_warnings = enable_crosswalk_warnings;
    domain_config.enable_lane_departure_warnings = enable_lane_departure_warnings;
    return domain_config;
}


QJsonObject SyncConfig::toJson() const {
    QJsonObject json;
    json["max_timestamp_diff_ms"] = max_timestamp_diff_ms;
    json["enable_sync_monitoring"] = enable_sync_monitoring;
    return json;
}

SyncConfig SyncConfig::fromJson(const QJsonObject& json) {
    SyncConfig config;

    if (json.contains("max_timestamp_diff_ms"))
        config.max_timestamp_diff_ms = json["max_timestamp_diff_ms"].toInt();

    if (json.contains("enable_sync_monitoring"))
        config.enable_sync_monitoring = json["enable_sync_monitoring"].toBool();

    return config;
}

QJsonObject AppConfig::toJson() const {
    QJsonObject json;
    json["network"] = network.toJson();
    json["video"] = video.toJson();
    json["warning"] = warning.toJson();
    json["sync"] = sync.toJson();
    return json;
}

AppConfig AppConfig::fromJson(const QJsonObject& json) {
    AppConfig config;

    if (json.contains("network"))
        config.network = NetworkConfig::fromJson(json["network"].toObject());

    if (json.contains("video"))
        config.video = VideoConfig::fromJson(json["video"].toObject());

    if (json.contains("warning"))
        config.warning = WarningConfig::fromJson(json["warning"].toObject());

    if (json.contains("sync"))
        config.sync = SyncConfig::fromJson(json["sync"].toObject());

    return config;
}

} // namespace config
