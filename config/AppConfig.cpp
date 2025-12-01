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
    if (json.contains("dashboard_ip"))
        config.host = json["dashboard_ip"].toString();

    if (json.contains("port"))
        config.port = static_cast<quint16>(json["port"].toInt());
    if (json.contains("dashboard_port"))
        config.port = static_cast<quint16>(json["dashboard_port"].toInt());

    // Fallbacks if config is empty/zero
    if (config.host.isEmpty())
        config.host = QStringLiteral("127.0.0.1");
    if (config.port == 0)
        config.port = 9000;

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
    if (json.contains("rtsp_url"))
        config.source_url = json["rtsp_url"].toString();

    if (json.contains("auto_start"))
        config.auto_start = json["auto_start"].toBool();

    if (config.source_url.isEmpty())
        config.source_url = QStringLiteral("udp://239.0.0.1:5000");

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
    json["video_processing"] = video_processing.toJson();
    json["warning"] = warning.toJson();
    json["sync"] = sync.toJson();
    json["ui"] = ui.toJson();
    return json;
}

AppConfig AppConfig::fromJson(const QJsonObject& json) {
    AppConfig config;

    if (json.contains("network"))
        config.network = NetworkConfig::fromJson(json["network"].toObject());

    if (json.contains("video"))
        config.video = VideoConfig::fromJson(json["video"].toObject());

    if (json.contains("video_processing"))
        config.video_processing = VideoProcessingConfig::fromJson(json["video_processing"].toObject());

    if (json.contains("warning"))
        config.warning = WarningConfig::fromJson(json["warning"].toObject());

    if (json.contains("sync"))
        config.sync = SyncConfig::fromJson(json["sync"].toObject());

    if (json.contains("ui"))
        config.ui = UIConfig::fromJson(json["ui"].toObject());

    return config;
}

// VideoProcessingConfig
QJsonObject VideoProcessingConfig::toJson() const {
    QJsonObject json;
    json["gst_bus_poll_interval_ms"] = gst_bus_poll_interval_ms;
    json["gst_state_change_timeout_multiplier"] = gst_state_change_timeout_multiplier;
    json["rtp_jitter_buffer_latency_ms"] = rtp_jitter_buffer_latency_ms;
    json["fps_calculation_interval_ms"] = fps_calculation_interval_ms;

    QJsonObject overlayJson;
    overlayJson["text_padding_x"] = overlay.text_padding_x;
    overlayJson["text_padding_y"] = overlay.text_padding_y;
    overlayJson["text_background_width"] = overlay.text_background_width;
    overlayJson["text_background_height"] = overlay.text_background_height;
    overlayJson["fitted_line_points_count"] = overlay.fitted_line_points_count;
    overlayJson["fitted_line_source_width"] = static_cast<double>(overlay.fitted_line_source_width);
    overlayJson["fitted_line_source_height"] = static_cast<double>(overlay.fitted_line_source_height);
    overlayJson["normal_opacity"] = static_cast<double>(overlay.normal_opacity);
    overlayJson["full_opacity"] = static_cast<double>(overlay.full_opacity);
    overlayJson["shape_alpha_low"] = overlay.shape_alpha_low;
    overlayJson["shape_alpha_high"] = overlay.shape_alpha_high;
    overlayJson["legacy_focal_length"] = static_cast<double>(overlay.legacy_focal_length);
    overlayJson["legacy_image_width"] = overlay.legacy_image_width;
    overlayJson["legacy_image_height"] = overlay.legacy_image_height;
    json["overlay"] = overlayJson;

    return json;
}

VideoProcessingConfig VideoProcessingConfig::fromJson(const QJsonObject& json) {
    VideoProcessingConfig config;

    if (json.contains("gst_bus_poll_interval_ms"))
        config.gst_bus_poll_interval_ms = json["gst_bus_poll_interval_ms"].toInt();
    if (json.contains("gst_state_change_timeout_multiplier"))
        config.gst_state_change_timeout_multiplier = json["gst_state_change_timeout_multiplier"].toInt();
    if (json.contains("rtp_jitter_buffer_latency_ms"))
        config.rtp_jitter_buffer_latency_ms = json["rtp_jitter_buffer_latency_ms"].toInt();
    if (json.contains("fps_calculation_interval_ms"))
        config.fps_calculation_interval_ms = json["fps_calculation_interval_ms"].toInt();

    if (json.contains("overlay")) {
        QJsonObject overlayJson = json["overlay"].toObject();
        if (overlayJson.contains("text_padding_x"))
            config.overlay.text_padding_x = overlayJson["text_padding_x"].toInt();
        if (overlayJson.contains("text_padding_y"))
            config.overlay.text_padding_y = overlayJson["text_padding_y"].toInt();
        if (overlayJson.contains("text_background_width"))
            config.overlay.text_background_width = overlayJson["text_background_width"].toInt();
        if (overlayJson.contains("text_background_height"))
            config.overlay.text_background_height = overlayJson["text_background_height"].toInt();
        if (overlayJson.contains("fitted_line_points_count"))
            config.overlay.fitted_line_points_count = overlayJson["fitted_line_points_count"].toInt();
        if (overlayJson.contains("fitted_line_source_width"))
            config.overlay.fitted_line_source_width = static_cast<float>(overlayJson["fitted_line_source_width"].toDouble());
        if (overlayJson.contains("fitted_line_source_height"))
            config.overlay.fitted_line_source_height = static_cast<float>(overlayJson["fitted_line_source_height"].toDouble());
        if (overlayJson.contains("normal_opacity"))
            config.overlay.normal_opacity = static_cast<float>(overlayJson["normal_opacity"].toDouble());
        if (overlayJson.contains("full_opacity"))
            config.overlay.full_opacity = static_cast<float>(overlayJson["full_opacity"].toDouble());
        if (overlayJson.contains("shape_alpha_low"))
            config.overlay.shape_alpha_low = overlayJson["shape_alpha_low"].toInt();
        if (overlayJson.contains("shape_alpha_high"))
            config.overlay.shape_alpha_high = overlayJson["shape_alpha_high"].toInt();
        if (overlayJson.contains("legacy_focal_length"))
            config.overlay.legacy_focal_length = static_cast<float>(overlayJson["legacy_focal_length"].toDouble());
        if (overlayJson.contains("legacy_image_width"))
            config.overlay.legacy_image_width = overlayJson["legacy_image_width"].toInt();
        if (overlayJson.contains("legacy_image_height"))
            config.overlay.legacy_image_height = overlayJson["legacy_image_height"].toInt();
    }

    return config;
}

// UIConfig
QJsonObject UIConfig::toJson() const {
    QJsonObject json;
    json["main_window_width"] = main_window_width;
    json["main_window_height"] = main_window_height;
    json["video_widget_min_width"] = video_widget_min_width;
    json["video_widget_min_height"] = video_widget_min_height;
    json["dashboard_widget_min_width"] = dashboard_widget_min_width;
    json["dashboard_widget_min_height"] = dashboard_widget_min_height;
    json["host_input_max_width"] = host_input_max_width;
    json["min_content_width"] = min_content_width;
    return json;
}

UIConfig UIConfig::fromJson(const QJsonObject& json) {
    UIConfig config;

    if (json.contains("main_window_width"))
        config.main_window_width = json["main_window_width"].toInt();
    if (json.contains("main_window_height"))
        config.main_window_height = json["main_window_height"].toInt();
    if (json.contains("video_widget_min_width"))
        config.video_widget_min_width = json["video_widget_min_width"].toInt();
    if (json.contains("video_widget_min_height"))
        config.video_widget_min_height = json["video_widget_min_height"].toInt();
    if (json.contains("dashboard_widget_min_width"))
        config.dashboard_widget_min_width = json["dashboard_widget_min_width"].toInt();
    if (json.contains("dashboard_widget_min_height"))
        config.dashboard_widget_min_height = json["dashboard_widget_min_height"].toInt();
    if (json.contains("host_input_max_width"))
        config.host_input_max_width = json["host_input_max_width"].toInt();
    if (json.contains("min_content_width"))
        config.min_content_width = json["min_content_width"].toInt();

    return config;
}

} // namespace config
