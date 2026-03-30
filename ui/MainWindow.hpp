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
#include <QSplitter>
#include <QFrame>
#include <QComboBox>

#include "AppController.hpp"
#include "DashboardWidget.hpp"

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
    QPushButton* video_start_button_;
    QPushButton* video_stop_button_;
    QLineEdit* host_input_;
    QLineEdit* port_input_;
    QComboBox* video_source_combo_;

    QSplitter* content_splitter_;
    DashboardWidget* dashboard_widget_;

    void setupUi();
    void setupMenuBar();
    void setupStatusBar();
    void setupControlPanel();
    void setupContentPanel();
    void setupConnections();

private slots:
    void onConnectButtonClicked();
    void onDisconnectButtonClicked();
    void onStatusMessageChanged(const QString& message);
    void onConnectionStateChanged(bool fully_connected);
    void onDataConnectionChanged(bool connected);
    void onVideoConnectionChanged(bool connected);

    void onAboutAction();
    void onExitAction();
    void onVideoSourceChanged(int index);
    void onVideoStartButtonClicked();
    void onVideoStopButtonClicked();
};

} // namespace ui
