#include "DashboardWidget.hpp"
#include <QQuickView>
#include <QQmlContext>
#include <QQmlEngine>
#include <QLibraryInfo>
#include <QVBoxLayout>
#include "AppController.hpp"
#include "LoggerMacros.hpp"

namespace ui {

DashboardWidget::DashboardWidget(QWidget* parent)
    : QWidget(parent) {

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    view_ = new QQuickView();
    view_->setResizeMode(QQuickView::SizeRootObjectToView);
    view_->setColor(QColor("#1a1a1a"));

    container_ = QWidget::createWindowContainer(view_, this);
    container_->setFocusPolicy(Qt::StrongFocus);
    container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container_->setMinimumSize(400, 300);
    setMinimumSize(400, 300);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(container_);

    connect(view_->engine(), &QQmlEngine::warnings, this, [](const QList<QQmlError>& warnings) {
        for (const auto& warning : warnings) {
            LOG_WARN << "QML: " << warning.toString().toStdString();
        }
    });
}

void DashboardWidget::setAppController(app::AppController* controller) {
    if (!controller) {
        LOG_ERROR << "AppController is null";
        return;
    }

    controller_ = controller;

    auto* context = view_->rootContext();
    context->setContextProperty("appController", controller);
    context->setContextProperty("laneViewModel", controller->laneViewModel());
    context->setContextProperty("markingModel", controller->markingListModel());
    context->setContextProperty("warningModel", controller->warningListModel());

    // Ensure standard import path is present (helps when system Qt is in a non-default location)
    const QString qmlPath = QLibraryInfo::path(QLibraryInfo::QmlImportsPath);
    if (!view_->engine()->importPathList().contains(qmlPath)) {
        view_->engine()->addImportPath(qmlPath);
    }

    LOG_INFO << "Loading QML Dashboard.qml from resources. Import paths:";
    for (const auto& path : view_->engine()->importPathList()) {
        LOG_INFO << "  " << path.toStdString();
    }

    LOG_INFO << "Loading QML Dashboard.qml from resources";
    view_->setSource(QUrl("qrc:/qml/Dashboard.qml"));

    if (view_->status() == QQuickView::Error) {
        LOG_ERROR << "Failed to load Dashboard.qml";
    }
}

}
