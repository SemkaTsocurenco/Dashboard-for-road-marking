#include "ConfigurationManager.hpp"
#include "LoggerMacros.hpp"
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>

namespace config {

AppConfig ConfigurationManager::loadFromFile(const QString& path) {
    LOG_INFO << "Loading configuration from: " << path.toStdString();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString error = QString("Failed to open config file: %1").arg(file.errorString());
        LOG_ERROR << error.toStdString();
        throw ConfigurationException(error);
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parse_error);

    if (parse_error.error != QJsonParseError::NoError) {
        QString error = QString("JSON parse error at offset %1: %2")
            .arg(parse_error.offset)
            .arg(parse_error.errorString());
        LOG_ERROR << error.toStdString();
        throw ConfigurationException(error);
    }

    if (!doc.isObject()) {
        QString error = "Config root must be a JSON object";
        LOG_ERROR << error.toStdString();
        throw ConfigurationException(error);
    }

    AppConfig config = AppConfig::fromJson(doc.object());

    QString validation_error;
    if (!validateConfig(config, validation_error)) {
        LOG_ERROR << "Configuration validation failed: " << validation_error.toStdString();
        throw ConfigurationException("Validation failed: " + validation_error);
    }

    LOG_INFO << "Configuration loaded and validated successfully";
    return config;
}

bool ConfigurationManager::saveToFile(const QString& path, const AppConfig& config) {
    LOG_INFO << "Saving configuration to: " << path.toStdString();

    QString validation_error;
    if (!validateConfig(config, validation_error)) {
        LOG_ERROR << "Cannot save invalid configuration: " << validation_error.toStdString();
        return false;
    }

    QJsonObject json = config.toJson();
    QJsonDocument doc(json);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        LOG_ERROR << "Failed to open file for writing: " << file.errorString().toStdString();
        return false;
    }

    QByteArray data = doc.toJson(QJsonDocument::Indented);
    qint64 written = file.write(data);
    file.close();

    if (written != data.size()) {
        LOG_ERROR << "Failed to write complete configuration";
        return false;
    }

    LOG_INFO << "Configuration saved successfully";
    return true;
}

AppConfig ConfigurationManager::defaultConfig() {
    LOG_INFO << "Creating default configuration";

    AppConfig config;
    LOG_DEBUG << "Default config: host=" << config.network.host.toStdString()
              << " port=" << config.network.port
              << " video_url=" << config.video.source_url.toStdString();

    return config;
}

bool ConfigurationManager::validateConfig(const AppConfig& config, QString& error) {
    if (!validateNetworkConfig(config.network, error))
        return false;

    if (!validateVideoConfig(config.video, error))
        return false;

    if (!validateWarningConfig(config.warning, error))
        return false;

    if (!validateSyncConfig(config.sync, error))
        return false;

    return true;
}

bool ConfigurationManager::validateNetworkConfig(const NetworkConfig& cfg, QString& error) {
    if (cfg.host.isEmpty()) {
        error = "Network host cannot be empty";
        return false;
    }

    if (cfg.port == 0) {
        error = "Network port must be between 1 and 65535";
        return false;
    }

    if (cfg.reconnect_interval_ms < 100 || cfg.reconnect_interval_ms > 60000) {
        error = "Reconnect interval must be between 100ms and 60000ms";
        return false;
    }

    if (cfg.max_reconnect_attempts < 0 || cfg.max_reconnect_attempts > 1000) {
        error = "Max reconnect attempts must be between 0 (unlimited) and 1000";
        return false;
    }

    return true;
}

bool ConfigurationManager::validateVideoConfig(const VideoConfig& cfg, QString& error) {
    if (cfg.source_url.isEmpty()) {
        error = "Video source URL cannot be empty";
        return false;
    }

    if (!cfg.source_url.contains("://")) {
        error = "Video source URL must contain protocol (e.g., rtsp://, http://)";
        return false;
    }

    return true;
}

bool ConfigurationManager::validateWarningConfig(const WarningConfig& cfg, QString& error) {
    if (cfg.lane_departure_threshold_m <= 0.0f) {
        error = "Lane departure threshold must be positive";
        return false;
    }

    if (cfg.crosswalk_distance_threshold_m <= 0.0f) {
        error = "Crosswalk distance threshold must be positive";
        return false;
    }

    if (cfg.crosswalk_critical_distance_m <= 0.0f) {
        error = "Crosswalk critical distance must be positive";
        return false;
    }

    if (cfg.crosswalk_critical_distance_m >= cfg.crosswalk_distance_threshold_m) {
        error = "Crosswalk critical distance must be less than distance threshold";
        return false;
    }

    if (cfg.min_marking_confidence > 100) {
        error = "Min marking confidence must be between 0 and 100";
        return false;
    }

    if (cfg.min_lane_quality > 100) {
        error = "Min lane quality must be between 0 and 100";
        return false;
    }

    return true;
}

bool ConfigurationManager::validateSyncConfig(const SyncConfig& cfg, QString& error) {
    if (cfg.max_timestamp_diff_ms < 0 || cfg.max_timestamp_diff_ms > 10000) {
        error = "Max timestamp diff must be between 0 and 10000ms";
        return false;
    }

    return true;
}

} // namespace config
