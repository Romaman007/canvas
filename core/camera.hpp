#pragma once
#include <cmath>
#include "types.hpp"

class Camera {
public:
    Camera();

    void zoomAt(double screenX, double screenY, double deltaExp);
    void panPx(double dx, double dy);
    void setOffsetPx(double x, double y);

    double scale() const { return std::exp2(zoomExp_); }
    double zoomExp() const { return zoomExp_; }
    Vec2 offsetPx() const { return offsetPx_; }
    Vec2 worldCenter() const;

    Vec2 worldFromScreen(double sx, double sy) const;
    Vec2 screenFromWorld(double wx, double wy) const;

    void rebase();
    bool needsRecenter() const;
    void shiftWorldCenter(const Vec2& delta);

private:
    double zoomExp_{0.0};
    Vec2   offsetPx_{0.0, 0.0};
    Vec2   worldCenter_{0.0, 0.0};
};
