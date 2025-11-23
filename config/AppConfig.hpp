#pragma once

#include <QString>
#include <QJsonObject>
#include <cstdint>

namespace domain {
    class WarningEngineConfig;
}

namespace config {


struct NetworkConfig {
    QString host{"127.0.0.1"};
    quint16 port{5000};
    int reconnect_interval_ms{5000};
    int max_reconnect_attempts{0};  // 0 = unlimited
    bool auto_reconnect{true};

    QJsonObject toJson() const;
    static NetworkConfig fromJson(const QJsonObject& json);
};


struct VideoConfig {
    QString source_url{"rtsp://127.0.0.1:8554/stream"};
    bool auto_start{false};

    QJsonObject toJson() const;
    static VideoConfig fromJson(const QJsonObject& json);
};


struct WarningConfig {
    float lane_departure_threshold_m{0.3f};
    float crosswalk_distance_threshold_m{30.0f};
    float crosswalk_critical_distance_m{10.0f};
    std::uint8_t min_marking_confidence{50};
    std::uint8_t min_lane_quality{60};
    bool enable_crosswalk_warnings{true};
    bool enable_lane_departure_warnings{true};

    QJsonObject toJson() const;
    static WarningConfig fromJson(const QJsonObject& json);
    domain::WarningEngineConfig toDomainConfig() const;
};


struct SyncConfig {
    int max_timestamp_diff_ms{500};  
    bool enable_sync_monitoring{true};

    QJsonObject toJson() const;
    static SyncConfig fromJson(const QJsonObject& json);
};


struct AppConfig {
    NetworkConfig network;
    VideoConfig video;
    WarningConfig warning;
    SyncConfig sync;

    QJsonObject toJson() const;
    static AppConfig fromJson(const QJsonObject& json);
};

} // namespace config
