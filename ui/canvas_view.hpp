#pragma once
#include <QWidget>
#include <memory>
#include "../core/camera.hpp"
#include "../core/scene.hpp"

class CanvasView : public QWidget {
    Q_OBJECT
public:
    explicit CanvasView(QWidget* parent = nullptr);
    ~CanvasView() override = default;

protected:
    void paintEvent(QPaintEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void keyReleaseEvent(QKeyEvent*) override;

private:
    enum class Mode { Navigate, Draw };

    void drawGrid(QPainter& p);
    void drawStrokes(QPainter& p);
    void drawHud(QPainter& p);
    QPointF toQt(const Vec2& v) const { return QPointF(v.x, v.y); }

private:
    Camera cam_;
    Scene  scene_;

    Mode   mode_      = Mode::Navigate;
    bool   panning_   = false;
    bool   spaceDown_ = false;
    QPointF lastPos_;

    double brushPx_   = 4.0;   // ← толщина кисти (экранные px), теперь поле объявлено
};
