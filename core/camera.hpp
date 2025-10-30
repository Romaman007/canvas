#pragma once
#include <cmath>
#include "types.hpp"  // Vec2

class Camera {
public:
    Camera();

    // Зум в точке курсора (экранные координаты, deltaExp — шаг в экспоненте, например 1/6 за тик колеса)
    void zoomAt(double screenX, double screenY, double deltaExp);

    // Панорамирование в пикселях экрана
    void panPx(double dx, double dy);

    // Установка смещения (в пикселях экрана)
    void setOffsetPx(double x, double y);

    // Текущий масштаб (screen_pixels_per_world_unit)
    double scale() const { return scale_; }

    // Текущее экранное смещение
    Vec2 offsetPx() const { return offsetPx_; }

    // Преобразования координат
    Vec2 worldFromScreen(double sx, double sy) const;
    Vec2 screenFromWorld(double wx, double wy) const;

    // Перенос системы координат, чтобы избегать потери точности на экстремальных масштабах
    void rebase();

private:
    double scale_{1.0};     // экранных пикселей на 1 world-единицу
    Vec2   offsetPx_{0, 0}; // смещение в пикселях экрана
    Vec2   worldCenter_{0, 0}; // «нулевая» точка мира, относительно которой считаем transform
};
