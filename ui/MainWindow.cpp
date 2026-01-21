#include "MainWindow.hpp"
#include "LoggerMacros.hpp"
#include "AppConfig.hpp"
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QTimer>

namespace ui {

MainWindow::MainWindow(app::AppController* controller, QWidget* parent)
    : QMainWindow(parent)
    , controller_(controller)
{
    LOG_INFO << "MainWindow created";
    setupUi();
    setupConnections();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    LOG_INFO << "MainWindow closing";
    controller_->shutdown();
    event->accept();
}


void MainWindow::setupUi()
{
    setWindowTitle("Dashboard - Road Marking Detector");
    const auto& ui_config = controller_->config().ui;
    resize(ui_config.main_window_width, ui_config.main_window_height);

    central_widget_ = new QWidget(this);
    setCentralWidget(central_widget_);

    main_layout_ = new QVBoxLayout(central_widget_);

    setupMenuBar();
    setupControlPanel();
    setupContentPanel();
    setupStatusBar();
}

void MainWindow::setupMenuBar()
{
    QMenu* file_menu = menuBar()->addMenu("&File");

    QAction* exit_action = file_menu->addAction("E&xit");
    exit_action->setShortcut(QKeySequence::Quit);
    connect(exit_action, &QAction::triggered, this, &MainWindow::onExitAction);

    QMenu* help_menu = menuBar()->addMenu("&Help");

    QAction* about_action = help_menu->addAction("&About");
    connect(about_action, &QAction::triggered, this, &MainWindow::onAboutAction);
}

void MainWindow::setupControlPanel()
{
    control_panel_ = new QGroupBox(this);
    control_panel_->setFlat(true);
    control_panel_->setTitle("");  // Remove title for more compact look

    auto* layout = new QHBoxLayout(control_panel_);
    layout->setContentsMargins(8, 6, 8, 6);  // Reduced margins
    layout->setSpacing(8);  // Reduced spacing

    const auto& ui_config = controller_->config().ui;

    // Host input group
    auto* host_label = new QLabel("Host:", control_panel_);
    host_label->setStyleSheet("font-weight: 600; color: #a5abb2;");
    layout->addWidget(host_label);

    host_input_ = new QLineEdit(controller_->config().network.host, control_panel_);
    host_input_->setMaximumWidth(ui_config.host_input_max_width);
    host_input_->setMinimumHeight(28);
    layout->addWidget(host_input_);

    // Port input group
    auto* port_label = new QLabel("Port:", control_panel_);
    port_label->setStyleSheet("font-weight: 600; color: #a5abb2;");
    layout->addWidget(port_label);

    port_input_ = new QLineEdit(QString::number(controller_->config().network.port), control_panel_);
    port_input_->setMaximumWidth(70);
    port_input_->setMinimumHeight(28);
    layout->addWidget(port_input_);

    // Visual separator
    auto* separator = new QFrame(control_panel_);
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Plain);
    separator->setStyleSheet("color: #313842;");
    layout->addWidget(separator);

    // Buttons
    connect_button_ = new QPushButton("Connect", control_panel_);
    connect_button_->setMinimumWidth(85);
    connect_button_->setMinimumHeight(28);
    connect(connect_button_, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    layout->addWidget(connect_button_);

    disconnect_button_ = new QPushButton("Disconnect", control_panel_);
    disconnect_button_->setMinimumWidth(85);
    disconnect_button_->setMinimumHeight(28);
    disconnect_button_->setEnabled(false);
    connect(disconnect_button_, &QPushButton::clicked, this, &MainWindow::onDisconnectButtonClicked);
    layout->addWidget(disconnect_button_);

    layout->addStretch();

    main_layout_->addWidget(control_panel_);
}

void MainWindow::setupContentPanel()
{
    content_splitter_ = new QSplitter(Qt::Horizontal, this);
    content_splitter_->setChildrenCollapsible(false);

    const auto& ui_config = controller_->config().ui;

    auto* video_widget = controller_->videoWidget();
    if (video_widget) {
        video_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        video_widget->setMinimumSize(ui_config.video_widget_min_width, ui_config.video_widget_min_height);
        content_splitter_->addWidget(video_widget);
    }

    dashboard_widget_ = new DashboardWidget(this);
    dashboard_widget_->setAppController(controller_);
    content_splitter_->addWidget(dashboard_widget_);

    // Set stretch factors for 60/40 split (video gets more space)
    content_splitter_->setStretchFactor(0, 3);  // Video: 60%
    content_splitter_->setStretchFactor(1, 2);  // 3D Dashboard: 40%

    // Force initial 60/40 split once layout is calculated
    QTimer::singleShot(0, this, [this, ui_config]() {
        if (content_splitter_) {
            const int total_width = this->width();
            const int video_width = qMax(ui_config.min_content_width, static_cast<int>(total_width * 0.6));
            const int dashboard_width = qMax(ui_config.min_content_width, static_cast<int>(total_width * 0.4));
            content_splitter_->setSizes({video_width, dashboard_width});
        }
    });

    main_layout_->addWidget(content_splitter_, 1);
}

void MainWindow::setupStatusBar()
{
    status_label_ = new QLabel("Ready", this);
    statusBar()->addWidget(status_label_, 1);

    data_status_label_ = new QLabel("Data: Disconnected", this);
    statusBar()->addPermanentWidget(data_status_label_);

    video_status_label_ = new QLabel("Video: Disconnected", this);
    statusBar()->addPermanentWidget(video_status_label_);

    sync_status_label_ = new QLabel("Sync: N/A", this);
    statusBar()->addPermanentWidget(sync_status_label_);
}


void MainWindow::setupConnections()
{

    connect(controller_, &app::AppController::statusMessageChanged,
            this, &MainWindow::onStatusMessageChanged);

    connect(controller_, &app::AppController::connectionStateChanged,
            this, &MainWindow::onConnectionStateChanged);

    connect(controller_, &app::AppController::dataConnectionChanged,
            this, &MainWindow::onDataConnectionChanged);

    connect(controller_, &app::AppController::videoConnectionChanged,
            this, &MainWindow::onVideoConnectionChanged);

    connect(controller_, &app::AppController::criticalError,
            this, [this](const QString& error) {
                QMessageBox::critical(this, "Critical Error", error);
            });
}


void MainWindow::onConnectButtonClicked()
{
    QString host = host_input_->text();
    bool ok;
    int port = port_input_->text().toInt(&ok);

    if (!ok || port <= 0 || port > config::NetworkConfig::MAX_PORT) {
        QMessageBox::warning(this, "Invalid Port",
            QString("Please enter a valid port number (1-%1)").arg(config::NetworkConfig::MAX_PORT));
        return;
    }

    LOG_INFO << "Connecting to " << host.toStdString() << ":" << port;

    controller_->connectionManager()->connectToHost(host, port);

    if (!controller_->isVideoConnected()) {
        controller_->videoWidget()->connectToSource();
    }
}

void MainWindow::onDisconnectButtonClicked()
{
    LOG_INFO << "Disconnecting...";
    controller_->connectionManager()->disconnectFromHost();
    controller_->videoWidget()->disconnectFromSource();
}

void MainWindow::onStatusMessageChanged(const QString& message)
{
    status_label_->setText(message);
}

void MainWindow::onConnectionStateChanged(bool fully_connected)
{
    if (fully_connected) {
        status_label_->setText("Fully Connected");
        status_label_->setStyleSheet("color: green; font-weight: bold;");
    } else {
        status_label_->setStyleSheet("");
    }
}

void MainWindow::onDataConnectionChanged(bool connected)
{
    if (connected) {
        data_status_label_->setText("Data: Connected");
        data_status_label_->setStyleSheet("color: green;");
        disconnect_button_->setEnabled(true);
    } else {
        data_status_label_->setText("Data: Disconnected");
        data_status_label_->setStyleSheet("color: red;");
        disconnect_button_->setEnabled(false);
    }
}

void MainWindow::onVideoConnectionChanged(bool connected)
{
    if (connected) {
        video_status_label_->setText("Video: Connected");
        video_status_label_->setStyleSheet("color: green;");
    } else {
        video_status_label_->setText("Video: Disconnected");
        video_status_label_->setStyleSheet("color: red;");
    }
}

void MainWindow::onAboutAction()
{
    QMessageBox::about(this, "About Dashboard",
        "<h3>Road Marking Detection Dashboard</h3>"
        "<p>Version 1.0</p>"
        "<p>A real-time dashboard for road marking detection and lane tracking.</p>"
        "<p>Built with Qt6 and C++17</p>");
}

void MainWindow::onExitAction()
{
    close();
}

} // namespace ui
