#pragma once

#include <QJsonObject>
#include <QColor>

namespace config {

struct CameraSettings {
    // Camera position
    float height{250.0f};
    float distance{-25.0f};
    float pitch_angle_deg{-35.0f};

    // Clipping planes
    float clip_near{0.1f};
    float clip_far{1000.0f};

    QJsonObject toJson() const;
    static CameraSettings fromJson(const QJsonObject& json);
};


struct LightingSettings {
    // Directional lights brightness
    float main_light_brightness{0.7f};
    float secondary_light_brightness{0.45f};
    float tertiary_light_brightness{0.25f};

    // Point light
    float point_light_y_position{10.0f};
    float point_light_brightness{5.0f};
    float point_light_constant_fade{1.0f};
    float point_light_linear_fade{0.1f};
    float point_light_quadratic_fade{0.5f};

    QJsonObject toJson() const;
    static LightingSettings fromJson(const QJsonObject& json);
};


struct RoadSettings {
    // Road dimensions
    float width_m{18.0f};
    float length_m{40.0f};
    float thickness_m{0.01f};

    // Road position
    float y_position{-0.05f};

    // Lane markings
    float edge_line_width{0.15f};
    float center_line_width{0.1f};
    int center_dash_count{12};
    float dash_length_ratio{0.5f};
    float line_y_offset{0.02f};

    // Material properties
    struct MaterialProperties {
        float metalness{0.0f};
        float roughness{0.9f};
        float specular{0.0f};
        float emissive{0.07f};
    } material;

    QJsonObject toJson() const;
    static RoadSettings fromJson(const QJsonObject& json);
};


struct MarkingVisualizationSettings {
    // Marking dimensions
    float y_position_above_plane{0.02f};
    float min_width_m{0.05f};
    float height_m{0.03f};
    float min_length_m{0.2f};

    // Opacity calculation
    float min_opacity{0.3f};
    float max_opacity{1.0f};
    float confidence_divisor{100.0f};

    // Material properties
    float emissive_factor_scale{0.8f};
    float roughness_solid{0.55f};
    float roughness_dashed{0.8f};

    QJsonObject toJson() const;
    static MarkingVisualizationSettings fromJson(const QJsonObject& json);
};


struct CarModelSettings {
    // Car elevation
    float y_elevation{0.6f};

    // Lane boundary markers
    float marker_scale{0.08f};
    float marker_y_position{0.1f};

    QJsonObject toJson() const;
    static CarModelSettings fromJson(const QJsonObject& json);
};


struct CenterOffsetIndicatorSettings {
    // Component dimensions
    int width{320};
    int height{130};

    // Offset limits
    float max_offset_m{1.0f};

    // Normalization for visualization
    float normalization_divisor{1.0f};
    float normalization_multiplier{2.0f};

    // Warning thresholds
    float safe_threshold_m{0.3f};
    float critical_threshold_m{0.6f};

    QJsonObject toJson() const;
    static CenterOffsetIndicatorSettings fromJson(const QJsonObject& json);
};


struct DashboardViewSettings {
    // View dimensions
    int width{800};
    int height{600};

    // Camera info box
    int camera_info_width{350};
    int camera_info_height{80};

    QJsonObject toJson() const;
    static DashboardViewSettings fromJson(const QJsonObject& json);
};


struct WarningPanelSettings {
    // Panel dimensions
    int width{360};

    // Animation
    int color_animation_duration_ms{150};

    // Opacity values
    float connected_opacity{1.0f};
    float disconnected_opacity{0.7f};
    float panel_opacity{0.8f};
    float overlay_opacity{0.92f};

    QJsonObject toJson() const;
    static WarningPanelSettings fromJson(const QJsonObject& json);
};


struct SceneConfig {
    CameraSettings camera;
    LightingSettings lighting;
    RoadSettings road;
    MarkingVisualizationSettings marking;
    CarModelSettings car;
    CenterOffsetIndicatorSettings center_offset;
    DashboardViewSettings dashboard;
    WarningPanelSettings warning_panel;

    QJsonObject toJson() const;
    static SceneConfig fromJson(const QJsonObject& json);
};

} // namespace config
