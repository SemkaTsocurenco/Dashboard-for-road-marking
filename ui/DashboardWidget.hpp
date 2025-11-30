#pragma once

#include <QWidget>
#include <QQuickView>

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
    QQuickView* view_{nullptr};
    QWidget* container_{nullptr};
};

}
