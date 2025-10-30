#include <QApplication>
#include <QMainWindow>
#include "canvas_view.hpp"

int main(int argc, char** argv){
    QApplication app(argc, argv);
    QMainWindow w;
    auto* view = new CanvasView(&w);
    w.setCentralWidget(view);
    w.resize(1200, 800);
    w.setWindowTitle("Infinite Canvas — zoom/pan/rebase MVP");
    w.show();
    return app.exec();
}
