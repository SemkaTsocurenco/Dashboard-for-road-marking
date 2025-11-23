#include "MainWindow.hpp"
#include "LoggerMacros.hpp"
#include <QMessageBox>
#include <QMenu>
#include <QAction>

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
    resize(1280, 720);

    central_widget_ = new QWidget(this);
    setCentralWidget(central_widget_);

    main_layout_ = new QVBoxLayout(central_widget_);

    setupMenuBar();
    setupControlPanel();
    setupVideoPanel();
    setupInfoPanels();
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
    control_panel_ = new QGroupBox("Connection Control", this);
    auto* layout = new QHBoxLayout(control_panel_);

    layout->addWidget(new QLabel("Host:", control_panel_));
    host_input_ = new QLineEdit(controller_->config().network.host, control_panel_);
    host_input_->setMaximumWidth(150);
    layout->addWidget(host_input_);

    layout->addWidget(new QLabel("Port:", control_panel_));
    port_input_ = new QLineEdit(QString::number(controller_->config().network.port), control_panel_);
    port_input_->setMaximumWidth(80);
    layout->addWidget(port_input_);

    connect_button_ = new QPushButton("Connect", control_panel_);
    connect(connect_button_, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    layout->addWidget(connect_button_);

    disconnect_button_ = new QPushButton("Disconnect", control_panel_);
    disconnect_button_->setEnabled(false);
    connect(disconnect_button_, &QPushButton::clicked, this, &MainWindow::onDisconnectButtonClicked);
    layout->addWidget(disconnect_button_);

    layout->addStretch();

    main_layout_->addWidget(control_panel_);
}

void MainWindow::setupVideoPanel()
{
    video_container_ = new QWidget(this);
    video_container_->setMinimumSize(640, 480);
    video_container_->setStyleSheet("background-color: black;");

    video_layout_ = new QVBoxLayout(video_container_);
    video_layout_->setContentsMargins(0, 0, 0, 0);

    auto* video_widget = controller_->videoWidget();
    if (video_widget) {
        video_layout_->addWidget(video_widget);
    } else {
        auto* placeholder = new QLabel("No Video Widget", video_container_);
        placeholder->setAlignment(Qt::AlignCenter);
        placeholder->setStyleSheet("color: white; font-size: 24px;");
        video_layout_->addWidget(placeholder);
    }

    main_layout_->addWidget(video_container_, 1); 
}

void MainWindow::setupInfoPanels()
{
    auto* info_layout = new QHBoxLayout();

    lane_info_panel_ = new QGroupBox("Lane State", this);
    auto* lane_layout = new QVBoxLayout(lane_info_panel_);

    lane_valid_label_ = new QLabel("Valid: No", lane_info_panel_);
    lane_width_label_ = new QLabel("Width: N/A", lane_info_panel_);
    lane_offset_label_ = new QLabel("Center Offset: N/A", lane_info_panel_);
    lane_quality_label_ = new QLabel("Quality: N/A", lane_info_panel_);

    lane_layout->addWidget(lane_valid_label_);
    lane_layout->addWidget(lane_width_label_);
    lane_layout->addWidget(lane_offset_label_);
    lane_layout->addWidget(lane_quality_label_);
    lane_layout->addStretch();

    info_layout->addWidget(lane_info_panel_);

    warning_panel_ = new QGroupBox("Warnings", this);
    auto* warning_layout = new QVBoxLayout(warning_panel_);
    auto* warning_label = new QLabel("No active warnings", warning_panel_);
    warning_layout->addWidget(warning_label);
    warning_layout->addStretch();

    info_layout->addWidget(warning_panel_);

    sync_info_panel_ = new QGroupBox("Synchronization", this);
    auto* sync_layout = new QVBoxLayout(sync_info_panel_);

    sync_diff_label_ = new QLabel("Timestamp Diff: N/A", sync_info_panel_);
    sync_status_label_info_ = new QLabel("Status: N/A", sync_info_panel_);

    sync_layout->addWidget(sync_diff_label_);
    sync_layout->addWidget(sync_status_label_info_);
    sync_layout->addStretch();

    info_layout->addWidget(sync_info_panel_);

    main_layout_->addLayout(info_layout);
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

    auto* lane_vm = controller_->laneViewModel();
    if (lane_vm) {
        connect(lane_vm, &viewmodels::LaneStateViewModel::validChanged,
                this, &MainWindow::onLaneStateChanged);
        connect(lane_vm, &viewmodels::LaneStateViewModel::laneWidthChanged,
                this, &MainWindow::onLaneStateChanged);
        connect(lane_vm, &viewmodels::LaneStateViewModel::centerOffsetChanged,
                this, &MainWindow::onLaneStateChanged);
        connect(lane_vm, &viewmodels::LaneStateViewModel::qualityChanged,
                this, &MainWindow::onLaneStateChanged);
    }

    auto* sync_monitor = controller_->syncMonitor();
    if (sync_monitor) {
        connect(sync_monitor, &app::SynchronizationMonitor::timestampDiffChanged,
                this, &MainWindow::onSyncStatusChanged);
        connect(sync_monitor, &app::SynchronizationMonitor::synchronizationChanged,
                this, &MainWindow::onSyncStatusChanged);
    }
}


void MainWindow::onConnectButtonClicked()
{
    QString host = host_input_->text();
    bool ok;
    int port = port_input_->text().toInt(&ok);

    if (!ok || port <= 0 || port > 65535) {
        QMessageBox::warning(this, "Invalid Port", "Please enter a valid port number (1-65535)");
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

void MainWindow::onLaneStateChanged()
{
    auto* lane_vm = controller_->laneViewModel();
    if (!lane_vm) return;

    lane_valid_label_->setText(QString("Valid: %1").arg(lane_vm->isValid() ? "Yes" : "No"));
    lane_width_label_->setText(QString("Width: %1 m").arg(lane_vm->laneWidthMeters(), 0, 'f', 2));
    lane_offset_label_->setText(QString("Center Offset: %1 m").arg(lane_vm->centerOffsetMeters(), 0, 'f', 2));
    lane_quality_label_->setText(QString("Quality: %1%").arg(lane_vm->qualityPercent()));

    if (lane_vm->isQualityGood()) {
        lane_quality_label_->setStyleSheet("color: green;");
    } else {
        lane_quality_label_->setStyleSheet("color: orange;");
    }
}

void MainWindow::onSyncStatusChanged()
{
    auto* sync_monitor = controller_->syncMonitor();
    if (!sync_monitor) return;

    sync_diff_label_->setText(QString("Timestamp Diff: %1 ms").arg(sync_monitor->timestampDiffMs()));

    if (sync_monitor->isSynchronized()) {
        sync_status_label_info_->setText("Status: Synchronized");
        sync_status_label_info_->setStyleSheet("color: green;");
        sync_status_label_->setText("Sync: OK");
        sync_status_label_->setStyleSheet("color: green;");
    } else {
        sync_status_label_info_->setText("Status: Desynchronized");
        sync_status_label_info_->setStyleSheet("color: red;");
        sync_status_label_->setText("Sync: WARN");
        sync_status_label_->setStyleSheet("color: red;");
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
