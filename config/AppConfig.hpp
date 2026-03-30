#pragma once

#include <QString>
#include <QJsonObject>
#include <cstdint>

namespace domain {
    class WarningEngineConfig;
}

namespace config {


struct NetworkConfig {
    // Connection settings
    QString host{"127.0.0.1"};
    quint16 port{9000};
    int reconnect_interval_ms{5000};
    int max_reconnect_attempts{0};  // 0 = unlimited
    bool auto_reconnect{true};

    // Validation constants
    static constexpr quint16 MAX_PORT = 65535;
    static constexpr int MIN_RECONNECT_INTERVAL_MS = 100;
    static constexpr int MAX_RECONNECT_INTERVAL_MS = 60000;
    static constexpr int MAX_RECONNECT_ATTEMPTS_LIMIT = 1000;

    // Default video streaming port (used for RTSP fallback)
    static constexpr quint16 DEFAULT_VIDEO_PORT = 5000;

    QJsonObject toJson() const;
    static NetworkConfig fromJson(const QJsonObject& json);
};


struct VideoConfig {
    QString source_url{"udp://239.0.0.1:5000"};
    QString camera_source_url{"csi://"};
    bool auto_start{false};

    QJsonObject toJson() const;
    static VideoConfig fromJson(const QJsonObject& json);
};


struct VideoProcessingConfig {
    // GStreamer settings
    int gst_bus_poll_interval_ms{250};
    int gst_state_change_timeout_multiplier{2};  // multiplied by GST_SECOND
    int rtp_jitter_buffer_latency_ms{50};

    // FPS calculation
    int fps_calculation_interval_ms{1000};

    // Overlay settings
    struct OverlaySettings {
        // Text overlay
        int text_padding_x{5};
        int text_padding_y{5};
        int text_background_width{250};
        int text_background_height{50};

        // Line rendering
        int fitted_line_points_count{100};

        // Source resolution for fitted lines (from Python)
        float fitted_line_source_width{640.0f};
        float fitted_line_source_height{480.0f};

        // Opacity values
        float normal_opacity{0.5f};
        float full_opacity{1.0f};
        int shape_alpha_low{100};
        int shape_alpha_high{120};

        // Legacy camera parameters (not used in projection)
        float legacy_focal_length{4500.0f};
        int legacy_image_width{1920};
        int legacy_image_height{1080};
    } overlay;

    QJsonObject toJson() const;
    static VideoProcessingConfig fromJson(const QJsonObject& json);
};


struct WarningConfig {
    float lane_departure_threshold_m{0.3f};
    float crosswalk_distance_threshold_m{30.0f};
    float crosswalk_critical_distance_m{10.0f};
    std::uint8_t min_marking_confidence{10};
    std::uint8_t min_lane_quality{10};
    bool enable_crosswalk_warnings{true};
    bool enable_lane_departure_warnings{true};

    QJsonObject toJson() const;
    static WarningConfig fromJson(const QJsonObject& json);
    domain::WarningEngineConfig toDomainConfig() const;
};


struct SyncConfig {
    int max_timestamp_diff_ms{500};
    bool enable_sync_monitoring{true};

    // Validation constants
    static constexpr int MAX_TIMESTAMP_DIFF_LIMIT_MS = 10000;

    QJsonObject toJson() const;
    static SyncConfig fromJson(const QJsonObject& json);
};


struct UIConfig {
    // Main window dimensions
    int main_window_width{1280};
    int main_window_height{720};

    // Widget minimum sizes
    int video_widget_min_width{400};
    int video_widget_min_height{300};
    int dashboard_widget_min_width{400};
    int dashboard_widget_min_height{300};

    // Control sizes
    int host_input_max_width{150};
    int min_content_width{400};

    QJsonObject toJson() const;
    static UIConfig fromJson(const QJsonObject& json);
};


struct ProtocolConfig {
    // Protocol constants
    static constexpr uint8_t PROTOCOL_VERSION = 0x02;
    static constexpr uint8_t SYNC_BYTE = 0xAA;
    static constexpr size_t MAX_PAYLOAD_LENGTH = 1024;
    static constexpr size_t HEADER_SIZE = 9;  // 1 + 1 + 1 + 4 + 2

    // Data conversion constants
    static constexpr int QUALITY_RAW_MAX = 255;
    static constexpr int QUALITY_PERCENTAGE_MAX = 100;
    static constexpr int CONFIDENCE_SCALE = 100;
};

struct AppConfig {
    NetworkConfig network;
    VideoConfig video;
    VideoProcessingConfig video_processing;
    WarningConfig warning;
    SyncConfig sync;
    UIConfig ui;

    QJsonObject toJson() const;
    static AppConfig fromJson(const QJsonObject& json);
};

} // namespace config
