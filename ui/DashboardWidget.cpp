#include "DashboardWidget.hpp"
#include <QQuickWidget>
#include <QQmlContext>
#include <QQmlEngine>
#include <QLibraryInfo>
#include <QVBoxLayout>
#include <QSurfaceFormat>
#include "AppController.hpp"
#include "LoggerMacros.hpp"

namespace ui {

DashboardWidget::DashboardWidget(QWidget* parent)
    : QWidget(parent) {

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    quickWidget_ = new QQuickWidget(this);

    // Set OpenGL surface format for Qt Quick3D
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples(4);
    quickWidget_->setFormat(format);

    quickWidget_->setResizeMode(QQuickWidget::SizeRootObjectToView);
    quickWidget_->setClearColor(QColor("#1a1a1a"));
    quickWidget_->setMinimumSize(400, 300);

    setMinimumSize(400, 300);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(quickWidget_);

    connect(quickWidget_->engine(), &QQmlEngine::warnings, this, [](const QList<QQmlError>& warnings) {
        for (const auto& warning : warnings) {
            LOG_WARN << "QML: " << warning.toString().toStdString();
        }
    });

    LOG_INFO << "QQuickWidget created with OpenGL format: "
             << format.majorVersion() << "." << format.minorVersion()
             << " (Core Profile)";
}

void DashboardWidget::setAppController(app::AppController* controller) {
    if (!controller) {
        LOG_ERROR << "AppController is null";
        return;
    }

    controller_ = controller;

    auto* context = quickWidget_->rootContext();
    context->setContextProperty("appController", controller);
    context->setContextProperty("laneViewModel", controller->laneViewModel());
    context->setContextProperty("markingModel", controller->markingListModel());
    context->setContextProperty("warningModel", controller->warningListModel());

    // Ensure standard import path is present (helps when system Qt is in a non-default location)
    const QString qmlPath = QLibraryInfo::path(QLibraryInfo::QmlImportsPath);
    if (!quickWidget_->engine()->importPathList().contains(qmlPath)) {
        quickWidget_->engine()->addImportPath(qmlPath);
    }

    LOG_INFO << "Loading QML Dashboard.qml from resources. Import paths:";
    for (const auto& path : quickWidget_->engine()->importPathList()) {
        LOG_INFO << "  " << path.toStdString();
    }

    LOG_INFO << "Loading QML Dashboard.qml from resources";
    quickWidget_->setSource(QUrl("qrc:/qml/Dashboard.qml"));

    if (quickWidget_->status() == QQuickWidget::Error) {
        LOG_ERROR << "Failed to load Dashboard.qml";
        auto errors = quickWidget_->errors();
        for (const auto& error : errors) {
            LOG_ERROR << "  " << error.toString().toStdString();
        }
    }
}

}
