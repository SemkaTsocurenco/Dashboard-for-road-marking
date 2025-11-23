#pragma once

#include "AppConfig.hpp"
#include <QString>
#include <stdexcept>

namespace config {

class ConfigurationException : public std::runtime_error {
public:
    explicit ConfigurationException(const QString& message)
        : std::runtime_error(message.toStdString())
        , message_(message)
    {}

    const QString& qmessage() const { return message_; }

private:
    QString message_;
};


class ConfigurationManager {
public:

    static AppConfig loadFromFile(const QString& path);
    static bool saveToFile(const QString& path, const AppConfig& config);
    static AppConfig defaultConfig();

private:
    
    static bool validateConfig(const AppConfig& config, QString& error);
    static bool validateNetworkConfig(const NetworkConfig& cfg, QString& error);
    static bool validateVideoConfig(const VideoConfig& cfg, QString& error);
    static bool validateWarningConfig(const WarningConfig& cfg, QString& error);
    static bool validateSyncConfig(const SyncConfig& cfg, QString& error);
};

} // namespace config
