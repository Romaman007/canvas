#include "camera.hpp"

Camera::Camera() = default;

void Camera::zoomAt(double screenX, double screenY, double deltaExp) {
    // мировая точка под курсором до изменения масштаба
    Vec2 before = worldFromScreen(screenX, screenY);

    // масштабируем экспоненциально (база 2)
    scale_ *= std::pow(2.0, deltaExp);

    // зажим диапазона масштаба и ребейз, чтобы не упираться в double/Qt точность
    if (scale_ > 1e10) {
        rebase();
        scale_ = 1e10;
    } else if (scale_ < 1e-9) {
        rebase();
        scale_ = 1e-9;
    }

    // мировая точка под курсором после изменения масштаба
    Vec2 after = worldFromScreen(screenX, screenY);

    // сохраняем положение курсора на экране (двигаем оффсет противоположно сдвигу мира)
    offsetPx_.x += (after.x - before.x) * scale_;
    offsetPx_.y += (after.y - before.y) * scale_;
}

void Camera::panPx(double dx, double dy) {
    offsetPx_.x += dx;
    offsetPx_.y += dy;
}

void Camera::setOffsetPx(double x, double y) {
    offsetPx_.x = x;
    offsetPx_.y = y;
}

Vec2 Camera::worldFromScreen(double sx, double sy) const {
    // (sx - offset) переводим в world, затем добавляем центр мира
    return Vec2{ (sx - offsetPx_.x) / scale_ + worldCenter_.x,
                 (sy - offsetPx_.y) / scale_ + worldCenter_.y };
}

Vec2 Camera::screenFromWorld(double wx, double wy) const {
    // (w - center) масштабируем в пиксели и добавляем offset
    return Vec2{ (wx - worldCenter_.x) * scale_ + offsetPx_.x,
                 (wy - worldCenter_.y) * scale_ + offsetPx_.y };
}

void Camera::rebase() {
    // переносим центр мира в текущую world-точку, соответствующую левому-верхнему экрану (0,0),
    // и обнуляем экранный оффсет — так координаты остаются «маленькими»
    Vec2 centerWorld = worldFromScreen(0.0, 0.0);
    worldCenter_ = centerWorld;
    offsetPx_ = {0.0, 0.0};
}
