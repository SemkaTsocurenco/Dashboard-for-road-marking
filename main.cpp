#include <QApplication>
#include <QMainWindow>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("Dashboard - Приборная панель детектора");
    window.resize(800, 600);

    QLabel *label = new QLabel("Dashboard готов к работе", &window);
    label->setAlignment(Qt::AlignCenter);
    window.setCentralWidget(label);

    window.show();

    return app.exec();
}
