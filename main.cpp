#include <QApplication>
#include "AppController.hpp"
#include "MainWindow.hpp"
#include "LoggerMacros.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Initialize logger
    logger::Logger::instance().set_level(logger::LogLevel::Debug);
    LOG_INFO << "=== Dashboard Application Starting ===";

    // Create and initialize AppController
    app::AppController controller;

    if (!controller.initialize("config.json")) {
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
