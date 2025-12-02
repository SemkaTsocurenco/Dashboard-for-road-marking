#include "SceneConfig.hpp"
#include "LoggerMacros.hpp"
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>

namespace config {

// SceneScaleSettings
QJsonObject SceneScaleSettings::toJson() const {
    QJsonObject json;
    json["scale_factor"] = static_cast<double>(scale_factor);
    return json;
}

SceneScaleSettings SceneScaleSettings::fromJson(const QJsonObject& json) {
    SceneScaleSettings settings;
    if (json.contains("scale_factor"))
        settings.scale_factor = static_cast<float>(json["scale_factor"].toDouble());
    return settings;
}

// CameraSettings
QJsonObject CameraSettings::toJson() const {
    QJsonObject json;
    json["height"] = static_cast<double>(height);
    json["distance"] = static_cast<double>(distance);
    json["pitch_angle_deg"] = static_cast<double>(pitch_angle_deg);
    json["clip_near"] = static_cast<double>(clip_near);
    json["clip_far"] = static_cast<double>(clip_far);
    return json;
}

CameraSettings CameraSettings::fromJson(const QJsonObject& json) {
    CameraSettings settings;
    if (json.contains("height"))
        settings.height = static_cast<float>(json["height"].toDouble());
    if (json.contains("distance"))
        settings.distance = static_cast<float>(json["distance"].toDouble());
    if (json.contains("pitch_angle_deg"))
        settings.pitch_angle_deg = static_cast<float>(json["pitch_angle_deg"].toDouble());
    if (json.contains("clip_near"))
        settings.clip_near = static_cast<float>(json["clip_near"].toDouble());
    if (json.contains("clip_far"))
        settings.clip_far = static_cast<float>(json["clip_far"].toDouble());
    return settings;
}

// LightingSettings
QJsonObject LightingSettings::toJson() const {
    QJsonObject json;
    json["main_light_brightness"] = static_cast<double>(main_light_brightness);
    json["secondary_light_brightness"] = static_cast<double>(secondary_light_brightness);
    json["tertiary_light_brightness"] = static_cast<double>(tertiary_light_brightness);
    json["point_light_y_position"] = static_cast<double>(point_light_y_position);
    json["point_light_brightness"] = static_cast<double>(point_light_brightness);
    json["point_light_constant_fade"] = static_cast<double>(point_light_constant_fade);
    json["point_light_linear_fade"] = static_cast<double>(point_light_linear_fade);
    json["point_light_quadratic_fade"] = static_cast<double>(point_light_quadratic_fade);
    return json;
}

LightingSettings LightingSettings::fromJson(const QJsonObject& json) {
    LightingSettings settings;
    if (json.contains("main_light_brightness"))
        settings.main_light_brightness = static_cast<float>(json["main_light_brightness"].toDouble());
    if (json.contains("secondary_light_brightness"))
        settings.secondary_light_brightness = static_cast<float>(json["secondary_light_brightness"].toDouble());
    if (json.contains("tertiary_light_brightness"))
        settings.tertiary_light_brightness = static_cast<float>(json["tertiary_light_brightness"].toDouble());
    if (json.contains("point_light_y_position"))
        settings.point_light_y_position = static_cast<float>(json["point_light_y_position"].toDouble());
    if (json.contains("point_light_brightness"))
        settings.point_light_brightness = static_cast<float>(json["point_light_brightness"].toDouble());
    if (json.contains("point_light_constant_fade"))
        settings.point_light_constant_fade = static_cast<float>(json["point_light_constant_fade"].toDouble());
    if (json.contains("point_light_linear_fade"))
        settings.point_light_linear_fade = static_cast<float>(json["point_light_linear_fade"].toDouble());
    if (json.contains("point_light_quadratic_fade"))
        settings.point_light_quadratic_fade = static_cast<float>(json["point_light_quadratic_fade"].toDouble());
    return settings;
}

// RoadSettings
QJsonObject RoadSettings::toJson() const {
    QJsonObject json;
    json["width_m"] = static_cast<double>(width_m);
    json["length_m"] = static_cast<double>(length_m);
    json["thickness_m"] = static_cast<double>(thickness_m);
    json["y_position"] = static_cast<double>(y_position);
    json["edge_line_width"] = static_cast<double>(edge_line_width);
    json["center_line_width"] = static_cast<double>(center_line_width);
    json["center_dash_count"] = center_dash_count;
    json["dash_length_ratio"] = static_cast<double>(dash_length_ratio);
    json["line_y_offset"] = static_cast<double>(line_y_offset);

    QJsonObject mat;
    mat["metalness"] = static_cast<double>(material.metalness);
    mat["roughness"] = static_cast<double>(material.roughness);
    mat["specular"] = static_cast<double>(material.specular);
    mat["emissive"] = static_cast<double>(material.emissive);
    json["material"] = mat;

    return json;
}

RoadSettings RoadSettings::fromJson(const QJsonObject& json) {
    RoadSettings settings;
    if (json.contains("width_m"))
        settings.width_m = static_cast<float>(json["width_m"].toDouble());
    if (json.contains("length_m"))
        settings.length_m = static_cast<float>(json["length_m"].toDouble());
    if (json.contains("thickness_m"))
        settings.thickness_m = static_cast<float>(json["thickness_m"].toDouble());
    if (json.contains("y_position"))
        settings.y_position = static_cast<float>(json["y_position"].toDouble());
    if (json.contains("edge_line_width"))
        settings.edge_line_width = static_cast<float>(json["edge_line_width"].toDouble());
    if (json.contains("center_line_width"))
        settings.center_line_width = static_cast<float>(json["center_line_width"].toDouble());
    if (json.contains("center_dash_count"))
        settings.center_dash_count = json["center_dash_count"].toInt();
    if (json.contains("dash_length_ratio"))
        settings.dash_length_ratio = static_cast<float>(json["dash_length_ratio"].toDouble());
    if (json.contains("line_y_offset"))
        settings.line_y_offset = static_cast<float>(json["line_y_offset"].toDouble());

    if (json.contains("material")) {
        QJsonObject mat = json["material"].toObject();
        if (mat.contains("metalness"))
            settings.material.metalness = static_cast<float>(mat["metalness"].toDouble());
        if (mat.contains("roughness"))
            settings.material.roughness = static_cast<float>(mat["roughness"].toDouble());
        if (mat.contains("specular"))
            settings.material.specular = static_cast<float>(mat["specular"].toDouble());
        if (mat.contains("emissive"))
            settings.material.emissive = static_cast<float>(mat["emissive"].toDouble());
    }

    return settings;
}

// MarkingVisualizationSettings
QJsonObject MarkingVisualizationSettings::toJson() const {
    QJsonObject json;
    json["y_position_above_plane"] = static_cast<double>(y_position_above_plane);
    json["min_width_m"] = static_cast<double>(min_width_m);
    json["height_m"] = static_cast<double>(height_m);
    json["min_length_m"] = static_cast<double>(min_length_m);
    json["min_opacity"] = static_cast<double>(min_opacity);
    json["max_opacity"] = static_cast<double>(max_opacity);
    json["confidence_divisor"] = static_cast<double>(confidence_divisor);
    json["emissive_factor_scale"] = static_cast<double>(emissive_factor_scale);
    json["roughness_solid"] = static_cast<double>(roughness_solid);
    json["roughness_dashed"] = static_cast<double>(roughness_dashed);
    return json;
}

MarkingVisualizationSettings MarkingVisualizationSettings::fromJson(const QJsonObject& json) {
    MarkingVisualizationSettings settings;
    if (json.contains("y_position_above_plane"))
        settings.y_position_above_plane = static_cast<float>(json["y_position_above_plane"].toDouble());
    if (json.contains("min_width_m"))
        settings.min_width_m = static_cast<float>(json["min_width_m"].toDouble());
    if (json.contains("height_m"))
        settings.height_m = static_cast<float>(json["height_m"].toDouble());
    if (json.contains("min_length_m"))
        settings.min_length_m = static_cast<float>(json["min_length_m"].toDouble());
    if (json.contains("min_opacity"))
        settings.min_opacity = static_cast<float>(json["min_opacity"].toDouble());
    if (json.contains("max_opacity"))
        settings.max_opacity = static_cast<float>(json["max_opacity"].toDouble());
    if (json.contains("confidence_divisor"))
        settings.confidence_divisor = static_cast<float>(json["confidence_divisor"].toDouble());
    if (json.contains("emissive_factor_scale"))
        settings.emissive_factor_scale = static_cast<float>(json["emissive_factor_scale"].toDouble());
    if (json.contains("roughness_solid"))
        settings.roughness_solid = static_cast<float>(json["roughness_solid"].toDouble());
    if (json.contains("roughness_dashed"))
        settings.roughness_dashed = static_cast<float>(json["roughness_dashed"].toDouble());
    return settings;
}

// CarModelSettings
QJsonObject CarModelSettings::toJson() const {
    QJsonObject json;
    json["y_elevation"] = static_cast<double>(y_elevation);
    json["marker_scale"] = static_cast<double>(marker_scale);
    json["marker_y_position"] = static_cast<double>(marker_y_position);
    return json;
}

CarModelSettings CarModelSettings::fromJson(const QJsonObject& json) {
    CarModelSettings settings;
    if (json.contains("y_elevation"))
        settings.y_elevation = static_cast<float>(json["y_elevation"].toDouble());
    if (json.contains("marker_scale"))
        settings.marker_scale = static_cast<float>(json["marker_scale"].toDouble());
    if (json.contains("marker_y_position"))
        settings.marker_y_position = static_cast<float>(json["marker_y_position"].toDouble());
    return settings;
}

// CenterOffsetIndicatorSettings
QJsonObject CenterOffsetIndicatorSettings::toJson() const {
    QJsonObject json;
    json["width"] = width;
    json["height"] = height;
    json["max_offset_m"] = static_cast<double>(max_offset_m);
    json["normalization_divisor"] = static_cast<double>(normalization_divisor);
    json["normalization_multiplier"] = static_cast<double>(normalization_multiplier);
    json["safe_threshold_m"] = static_cast<double>(safe_threshold_m);
    json["critical_threshold_m"] = static_cast<double>(critical_threshold_m);
    return json;
}

CenterOffsetIndicatorSettings CenterOffsetIndicatorSettings::fromJson(const QJsonObject& json) {
    CenterOffsetIndicatorSettings settings;
    if (json.contains("width"))
        settings.width = json["width"].toInt();
    if (json.contains("height"))
        settings.height = json["height"].toInt();
    if (json.contains("max_offset_m"))
        settings.max_offset_m = static_cast<float>(json["max_offset_m"].toDouble());
    if (json.contains("normalization_divisor"))
        settings.normalization_divisor = static_cast<float>(json["normalization_divisor"].toDouble());
    if (json.contains("normalization_multiplier"))
        settings.normalization_multiplier = static_cast<float>(json["normalization_multiplier"].toDouble());
    if (json.contains("safe_threshold_m"))
        settings.safe_threshold_m = static_cast<float>(json["safe_threshold_m"].toDouble());
    if (json.contains("critical_threshold_m"))
        settings.critical_threshold_m = static_cast<float>(json["critical_threshold_m"].toDouble());
    return settings;
}

// DashboardViewSettings
QJsonObject DashboardViewSettings::toJson() const {
    QJsonObject json;
    json["width"] = width;
    json["height"] = height;
    json["camera_info_width"] = camera_info_width;
    json["camera_info_height"] = camera_info_height;
    return json;
}

DashboardViewSettings DashboardViewSettings::fromJson(const QJsonObject& json) {
    DashboardViewSettings settings;
    if (json.contains("width"))
        settings.width = json["width"].toInt();
    if (json.contains("height"))
        settings.height = json["height"].toInt();
    if (json.contains("camera_info_width"))
        settings.camera_info_width = json["camera_info_width"].toInt();
    if (json.contains("camera_info_height"))
        settings.camera_info_height = json["camera_info_height"].toInt();
    return settings;
}

// WarningPanelSettings
QJsonObject WarningPanelSettings::toJson() const {
    QJsonObject json;
    json["width"] = width;
    json["color_animation_duration_ms"] = color_animation_duration_ms;
    json["connected_opacity"] = static_cast<double>(connected_opacity);
    json["disconnected_opacity"] = static_cast<double>(disconnected_opacity);
    json["panel_opacity"] = static_cast<double>(panel_opacity);
    json["overlay_opacity"] = static_cast<double>(overlay_opacity);
    return json;
}

WarningPanelSettings WarningPanelSettings::fromJson(const QJsonObject& json) {
    WarningPanelSettings settings;
    if (json.contains("width"))
        settings.width = json["width"].toInt();
    if (json.contains("color_animation_duration_ms"))
        settings.color_animation_duration_ms = json["color_animation_duration_ms"].toInt();
    if (json.contains("connected_opacity"))
        settings.connected_opacity = static_cast<float>(json["connected_opacity"].toDouble());
    if (json.contains("disconnected_opacity"))
        settings.disconnected_opacity = static_cast<float>(json["disconnected_opacity"].toDouble());
    if (json.contains("panel_opacity"))
        settings.panel_opacity = static_cast<float>(json["panel_opacity"].toDouble());
    if (json.contains("overlay_opacity"))
        settings.overlay_opacity = static_cast<float>(json["overlay_opacity"].toDouble());
    return settings;
}

// SceneConfig
QJsonObject SceneConfig::toJson() const {
    QJsonObject json;
    json["scale"] = scale.toJson();
    json["camera"] = camera.toJson();
    json["lighting"] = lighting.toJson();
    json["road"] = road.toJson();
    json["marking"] = marking.toJson();
    json["car"] = car.toJson();
    json["center_offset"] = center_offset.toJson();
    json["dashboard"] = dashboard.toJson();
    json["warning_panel"] = warning_panel.toJson();
    return json;
}

SceneConfig SceneConfig::fromJson(const QJsonObject& json) {
    SceneConfig config;
    if (json.contains("scale"))
        config.scale = SceneScaleSettings::fromJson(json["scale"].toObject());
    if (json.contains("camera"))
        config.camera = CameraSettings::fromJson(json["camera"].toObject());
    if (json.contains("lighting"))
        config.lighting = LightingSettings::fromJson(json["lighting"].toObject());
    if (json.contains("road"))
        config.road = RoadSettings::fromJson(json["road"].toObject());
    if (json.contains("marking"))
        config.marking = MarkingVisualizationSettings::fromJson(json["marking"].toObject());
    if (json.contains("car"))
        config.car = CarModelSettings::fromJson(json["car"].toObject());
    if (json.contains("center_offset"))
        config.center_offset = CenterOffsetIndicatorSettings::fromJson(json["center_offset"].toObject());
    if (json.contains("dashboard"))
        config.dashboard = DashboardViewSettings::fromJson(json["dashboard"].toObject());
    if (json.contains("warning_panel"))
        config.warning_panel = WarningPanelSettings::fromJson(json["warning_panel"].toObject());
    return config;
}

SceneConfig SceneConfig::loadFromFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG_WARN << "Failed to load scene config from " << path.toStdString() << ", using defaults";
        return SceneConfig(); // Return default config
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parse_error);

    if (parse_error.error != QJsonParseError::NoError) {
        LOG_WARN << "Scene config JSON parse error: " << parse_error.errorString().toStdString();
        return SceneConfig(); // Return default config
    }

    if (!doc.isObject()) {
        LOG_WARN << "Scene config root must be a JSON object";
        return SceneConfig(); // Return default config
    }

    LOG_INFO << "Scene config loaded successfully from " << path.toStdString();
    return fromJson(doc.object());
}

} // namespace config
