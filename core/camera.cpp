#include "camera.hpp"
#include <algorithm>

namespace {
constexpr double kMaxZoomExp = 45.0;
constexpr double kMinZoomExp = -55.0;
constexpr double kRebaseZoomThreshold = 1.0;
}

Camera::Camera() = default;

Vec2 Camera::worldFromScreen(double sx, double sy) const {
    const double sc = scale();
    return Vec2{
        (sx - offsetPx_.x) / sc + worldCenter_.x,
        (sy - offsetPx_.y) / sc + worldCenter_.y
    };
}

Vec2 Camera::screenFromWorld(double wx, double wy) const {
    const double sc = scale();
    return Vec2{
        (wx - worldCenter_.x) * sc + offsetPx_.x,
        (wy - worldCenter_.y) * sc + offsetPx_.y
    };
}

Vec2 Camera::worldCenter() const {
    return worldCenter_;
}

void Camera::zoomAt(double screenX, double screenY, double deltaExp) {
    Vec2 before = worldFromScreen(screenX, screenY);

    zoomExp_ = std::clamp(zoomExp_ + deltaExp, kMinZoomExp, kMaxZoomExp);

    Vec2 after = worldFromScreen(screenX, screenY);
    const double sc = scale();
    offsetPx_.x += (after.x - before.x) * sc;
    offsetPx_.y += (after.y - before.y) * sc;

    if (std::abs(zoomExp_) > kRebaseZoomThreshold) {
        rebase();
    }
}

void Camera::panPx(double dx, double dy) {
    offsetPx_.x += dx;
    offsetPx_.y += dy;
}

void Camera::setOffsetPx(double x, double y) {
    offsetPx_.x = x;
    offsetPx_.y = y;
}

bool Camera::needsRecenter() const {
    const double threshold = 1e6;
    return std::fabs(worldCenter_.x) > threshold || std::fabs(worldCenter_.y) > threshold;
}

void Camera::shiftWorldCenter(const Vec2& delta) {
    worldCenter_.x -= delta.x;
    worldCenter_.y -= delta.y;
}

void Camera::rebase() {
    Vec2 centerWorld = worldFromScreen(0.0, 0.0);
    worldCenter_ = centerWorld;
    offsetPx_ = {0.0, 0.0};
}
