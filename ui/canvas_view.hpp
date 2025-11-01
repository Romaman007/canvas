#pragma once
#include <QWidget>
#include <QColor>
#include <cstdint>
#include "../core/camera.hpp"
#include "tool_mode.hpp"

class Scene;

class CanvasView : public QWidget {
    Q_OBJECT
public:
    explicit CanvasView(Scene* scene, QWidget* parent = nullptr);
    ~CanvasView() override = default;

    void setMode(ui::Mode mode);
    ui::Mode mode() const { return mode_; }

    void setBrushWidth(double px);
    double brushWidth() const { return brushPx_; }
    void setBrushColor(const QColor& color);
    QColor brushColor() const;

protected:
    void paintEvent(QPaintEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void keyReleaseEvent(QKeyEvent*) override;

signals:
    void modeChanged(ui::Mode mode);
    void brushWidthChanged(double width);
    void brushColorChanged(const QColor& color);

private:
    void drawGrid(QPainter& p);
    void drawStrokes(QPainter& p);
    void drawHud(QPainter& p);
    QPointF toQt(const Vec2& v) const { return QPointF(v.x, v.y); }
    void recenterSceneIfNeeded();

private:
    Camera cam_;
    Scene* scene_{nullptr};

    ui::Mode mode_      = ui::Mode::Pan;
    bool     panning_   = false;
    bool     spaceDown_ = false;
    QPointF  lastPos_;

    double brushPx_ = 4.0; // Default brush width in pixels.
    std::uint32_t brushColorRGB_ = 0xE6E6E6; // Light grey by default.
};
