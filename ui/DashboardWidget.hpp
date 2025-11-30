#pragma once

#include <QWidget>
#include <QQuickWidget>

namespace app {
class AppController;
}

namespace ui {

class DashboardWidget : public QWidget {
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget* parent = nullptr);
    void setAppController(app::AppController* controller);

private:
    app::AppController* controller_{nullptr};
    QQuickWidget* quickWidget_{nullptr};
};

}
