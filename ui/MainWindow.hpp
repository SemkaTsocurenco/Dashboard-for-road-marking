#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QStatusBar>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QCloseEvent>

#include "AppController.hpp"

namespace ui {

/**
 * @brief Main window of the Dashboard application

 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(app::AppController* controller, QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    app::AppController* controller_;

    QWidget* central_widget_;
    QVBoxLayout* main_layout_;

    QLabel* status_label_;
    QLabel* data_status_label_;
    QLabel* video_status_label_;
    QLabel* sync_status_label_;

    QGroupBox* control_panel_;
    QPushButton* connect_button_;
    QPushButton* disconnect_button_;
    QLineEdit* host_input_;
    QLineEdit* port_input_;

    QWidget* video_container_;
    QVBoxLayout* video_layout_;

    QGroupBox* lane_info_panel_;
    QGroupBox* warning_panel_;
    QGroupBox* sync_info_panel_;

    QLabel* lane_valid_label_;
    QLabel* lane_width_label_;
    QLabel* lane_offset_label_;
    QLabel* lane_quality_label_;

    QLabel* sync_diff_label_;
    QLabel* sync_status_label_info_;

    void setupUi();
    void setupMenuBar();
    void setupStatusBar();
    void setupControlPanel();
    void setupVideoPanel();
    void setupInfoPanels();
    void setupConnections();

private slots:
    void onConnectButtonClicked();
    void onDisconnectButtonClicked();
    void onStatusMessageChanged(const QString& message);
    void onConnectionStateChanged(bool fully_connected);
    void onDataConnectionChanged(bool connected);
    void onVideoConnectionChanged(bool connected);
    void onLaneStateChanged();
    void onSyncStatusChanged();

    void onAboutAction();
    void onExitAction();
};

} // namespace ui
