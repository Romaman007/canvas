#include <QApplication>
#include "canvas_window.hpp"

int main(int argc, char** argv){
    QApplication app(argc, argv);
    CanvasWindow w;
    w.resize(1200, 800);
    w.setWindowTitle(QStringLiteral("Infinite Canvas - zoom/pan MVP"));
    w.show();
    return app.exec();
}
