#pragma once

#include <QObject>
#include <qqml.h>
#include "SceneConfig.hpp"

namespace viewmodels {

class SceneConfigViewModel : public QObject {
    Q_OBJECT
    QML_ELEMENT

    // Camera properties
    Q_PROPERTY(float cameraHeight READ cameraHeight CONSTANT)
    Q_PROPERTY(float cameraDistance READ cameraDistance CONSTANT)
    Q_PROPERTY(float cameraPitchAngle READ cameraPitchAngle CONSTANT)
    Q_PROPERTY(float cameraClipNear READ cameraClipNear CONSTANT)
    Q_PROPERTY(float cameraClipFar READ cameraClipFar CONSTANT)

    // Lighting properties
    Q_PROPERTY(float mainLightBrightness READ mainLightBrightness CONSTANT)
    Q_PROPERTY(float secondaryLightBrightness READ secondaryLightBrightness CONSTANT)
    Q_PROPERTY(float tertiaryLightBrightness READ tertiaryLightBrightness CONSTANT)
    Q_PROPERTY(float pointLightYPosition READ pointLightYPosition CONSTANT)
    Q_PROPERTY(float pointLightBrightness READ pointLightBrightness CONSTANT)
    Q_PROPERTY(float pointLightConstantFade READ pointLightConstantFade CONSTANT)
    Q_PROPERTY(float pointLightLinearFade READ pointLightLinearFade CONSTANT)
    Q_PROPERTY(float pointLightQuadraticFade READ pointLightQuadraticFade CONSTANT)

    // Road properties
    Q_PROPERTY(float roadWidth READ roadWidth CONSTANT)
    Q_PROPERTY(float roadLength READ roadLength CONSTANT)
    Q_PROPERTY(float roadThickness READ roadThickness CONSTANT)
    Q_PROPERTY(float roadYPosition READ roadYPosition CONSTANT)
    Q_PROPERTY(float edgeLineWidth READ edgeLineWidth CONSTANT)
    Q_PROPERTY(float centerLineWidth READ centerLineWidth CONSTANT)
    Q_PROPERTY(int centerDashCount READ centerDashCount CONSTANT)
    Q_PROPERTY(float dashLengthRatio READ dashLengthRatio CONSTANT)
    Q_PROPERTY(float lineYOffset READ lineYOffset CONSTANT)
    Q_PROPERTY(float roadMetalness READ roadMetalness CONSTANT)
    Q_PROPERTY(float roadRoughness READ roadRoughness CONSTANT)
    Q_PROPERTY(float roadSpecular READ roadSpecular CONSTANT)
    Q_PROPERTY(float roadEmissive READ roadEmissive CONSTANT)

    // Marking visualization properties
    Q_PROPERTY(float markingYPosition READ markingYPosition CONSTANT)
    Q_PROPERTY(float markingMinWidth READ markingMinWidth CONSTANT)
    Q_PROPERTY(float markingHeight READ markingHeight CONSTANT)
    Q_PROPERTY(float markingMinLength READ markingMinLength CONSTANT)
    Q_PROPERTY(float markingMinOpacity READ markingMinOpacity CONSTANT)
    Q_PROPERTY(float markingMaxOpacity READ markingMaxOpacity CONSTANT)
    Q_PROPERTY(float markingConfidenceDivisor READ markingConfidenceDivisor CONSTANT)
    Q_PROPERTY(float markingEmissiveFactorScale READ markingEmissiveFactorScale CONSTANT)
    Q_PROPERTY(float markingRoughnessSolid READ markingRoughnessSolid CONSTANT)
    Q_PROPERTY(float markingRoughnessDashed READ markingRoughnessDashed CONSTANT)

    // Car model properties
    Q_PROPERTY(float carYElevation READ carYElevation CONSTANT)
    Q_PROPERTY(float markerScale READ markerScale CONSTANT)
    Q_PROPERTY(float markerYPosition READ markerYPosition CONSTANT)

    // Center offset indicator properties
    Q_PROPERTY(int centerOffsetWidth READ centerOffsetWidth CONSTANT)
    Q_PROPERTY(int centerOffsetHeight READ centerOffsetHeight CONSTANT)
    Q_PROPERTY(float maxOffsetM READ maxOffsetM CONSTANT)
    Q_PROPERTY(float normalizationDivisor READ normalizationDivisor CONSTANT)
    Q_PROPERTY(float normalizationMultiplier READ normalizationMultiplier CONSTANT)
    Q_PROPERTY(float safeThresholdM READ safeThresholdM CONSTANT)
    Q_PROPERTY(float criticalThresholdM READ criticalThresholdM CONSTANT)

    // Dashboard view properties
    Q_PROPERTY(int dashboardWidth READ dashboardWidth CONSTANT)
    Q_PROPERTY(int dashboardHeight READ dashboardHeight CONSTANT)
    Q_PROPERTY(int cameraInfoWidth READ cameraInfoWidth CONSTANT)
    Q_PROPERTY(int cameraInfoHeight READ cameraInfoHeight CONSTANT)

    // Warning panel properties
    Q_PROPERTY(int warningPanelWidth READ warningPanelWidth CONSTANT)
    Q_PROPERTY(int colorAnimationDuration READ colorAnimationDuration CONSTANT)
    Q_PROPERTY(float connectedOpacity READ connectedOpacity CONSTANT)
    Q_PROPERTY(float disconnectedOpacity READ disconnectedOpacity CONSTANT)
    Q_PROPERTY(float panelOpacity READ panelOpacity CONSTANT)
    Q_PROPERTY(float overlayOpacity READ overlayOpacity CONSTANT)

public:
    explicit SceneConfigViewModel(QObject* parent = nullptr);
    explicit SceneConfigViewModel(const config::SceneConfig& config, QObject* parent = nullptr);

    // Camera getters
    float cameraHeight() const { return config_.camera.height; }
    float cameraDistance() const { return config_.camera.distance; }
    float cameraPitchAngle() const { return config_.camera.pitch_angle_deg; }
    float cameraClipNear() const { return config_.camera.clip_near; }
    float cameraClipFar() const { return config_.camera.clip_far; }

    // Lighting getters
    float mainLightBrightness() const { return config_.lighting.main_light_brightness; }
    float secondaryLightBrightness() const { return config_.lighting.secondary_light_brightness; }
    float tertiaryLightBrightness() const { return config_.lighting.tertiary_light_brightness; }
    float pointLightYPosition() const { return config_.lighting.point_light_y_position; }
    float pointLightBrightness() const { return config_.lighting.point_light_brightness; }
    float pointLightConstantFade() const { return config_.lighting.point_light_constant_fade; }
    float pointLightLinearFade() const { return config_.lighting.point_light_linear_fade; }
    float pointLightQuadraticFade() const { return config_.lighting.point_light_quadratic_fade; }

    // Road getters
    float roadWidth() const { return config_.road.width_m; }
    float roadLength() const { return config_.road.length_m; }
    float roadThickness() const { return config_.road.thickness_m; }
    float roadYPosition() const { return config_.road.y_position; }
    float edgeLineWidth() const { return config_.road.edge_line_width; }
    float centerLineWidth() const { return config_.road.center_line_width; }
    int centerDashCount() const { return config_.road.center_dash_count; }
    float dashLengthRatio() const { return config_.road.dash_length_ratio; }
    float lineYOffset() const { return config_.road.line_y_offset; }
    float roadMetalness() const { return config_.road.material.metalness; }
    float roadRoughness() const { return config_.road.material.roughness; }
    float roadSpecular() const { return config_.road.material.specular; }
    float roadEmissive() const { return config_.road.material.emissive; }

    // Marking getters
    float markingYPosition() const { return config_.marking.y_position_above_plane; }
    float markingMinWidth() const { return config_.marking.min_width_m; }
    float markingHeight() const { return config_.marking.height_m; }
    float markingMinLength() const { return config_.marking.min_length_m; }
    float markingMinOpacity() const { return config_.marking.min_opacity; }
    float markingMaxOpacity() const { return config_.marking.max_opacity; }
    float markingConfidenceDivisor() const { return config_.marking.confidence_divisor; }
    float markingEmissiveFactorScale() const { return config_.marking.emissive_factor_scale; }
    float markingRoughnessSolid() const { return config_.marking.roughness_solid; }
    float markingRoughnessDashed() const { return config_.marking.roughness_dashed; }

    // Car getters
    float carYElevation() const { return config_.car.y_elevation; }
    float markerScale() const { return config_.car.marker_scale; }
    float markerYPosition() const { return config_.car.marker_y_position; }

    // Center offset getters
    int centerOffsetWidth() const { return config_.center_offset.width; }
    int centerOffsetHeight() const { return config_.center_offset.height; }
    float maxOffsetM() const { return config_.center_offset.max_offset_m; }
    float normalizationDivisor() const { return config_.center_offset.normalization_divisor; }
    float normalizationMultiplier() const { return config_.center_offset.normalization_multiplier; }
    float safeThresholdM() const { return config_.center_offset.safe_threshold_m; }
    float criticalThresholdM() const { return config_.center_offset.critical_threshold_m; }

    // Dashboard getters
    int dashboardWidth() const { return config_.dashboard.width; }
    int dashboardHeight() const { return config_.dashboard.height; }
    int cameraInfoWidth() const { return config_.dashboard.camera_info_width; }
    int cameraInfoHeight() const { return config_.dashboard.camera_info_height; }

    // Warning panel getters
    int warningPanelWidth() const { return config_.warning_panel.width; }
    int colorAnimationDuration() const { return config_.warning_panel.color_animation_duration_ms; }
    float connectedOpacity() const { return config_.warning_panel.connected_opacity; }
    float disconnectedOpacity() const { return config_.warning_panel.disconnected_opacity; }
    float panelOpacity() const { return config_.warning_panel.panel_opacity; }
    float overlayOpacity() const { return config_.warning_panel.overlay_opacity; }

private:
    config::SceneConfig config_;
};

} // namespace viewmodels
