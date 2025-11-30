#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include "AppController.hpp"
#include "MainWindow.hpp"
#include "LoggerMacros.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Initialize logger
    logger::Logger::instance().set_level(logger::LogLevel::Debug);
    LOG_INFO << "=== Dashboard Application Starting ===";

    // Parse CLI overrides
    QCommandLineParser parser;
    parser.setApplicationDescription("Dashboard - Road Marking Detector");
    parser.addHelpOption();

    QCommandLineOption hostOption(QStringList() << "H" << "host",
                                  "Dashboard host / video host (default 127.0.0.1)",
                                  "host");
    QCommandLineOption portOption(QStringList() << "P" << "port",
                                  "Dashboard port / video port (default 5000)",
                                  "port");

    parser.addOption(hostOption);
    parser.addOption(portOption);
    parser.process(app);

    QString override_host = parser.value(hostOption);
    quint16 override_port = 0;
    if (parser.isSet(portOption)) {
        bool ok = false;
        int p = parser.value(portOption).toInt(&ok);
        if (ok && p > 0 && p <= 65535) {
            override_port = static_cast<quint16>(p);
        } else {
            LOG_WARN << "Invalid --port value, ignoring override";
        }
    }

    // Create and initialize AppController
    app::AppController controller;

    if (!controller.initialize("config.json", override_host, override_port)) {
        LOG_ERROR << "Failed to initialize AppController";
        return 1;
    }

    // Create main window with controller
    ui::MainWindow window(&controller);
    window.show();

    LOG_INFO << "Application ready, entering event loop";

    int result = app.exec();

    LOG_INFO << "=== Dashboard Application Exiting with code: " << result << " ===";
    return result;
}
