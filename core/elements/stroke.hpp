#pragma once
#include <vector>
#include <cstddef>
#include "../types.hpp"

class Camera; // forward-declare, чтобы не тянуть заголовки лишний раз

class Stroke {
public:
    Stroke() = default;

    // Инициализируем штрих: кисть в пикселях экрана → фиксируем ширину в мире
    void begin(double brushPx, const Camera& cam);

    // Добавляем точку, заданную в экранных координатах.
    // minStepPx — минимальный шаг по экрану между соседними записями,
    // чтобы не сыпать лишние точки при сильном зуме (например, 1.5–2.0 px)
    void addScreenPoint(double sx, double sy, const Camera& cam, double minStepPx = 1.5);

    // Завершение (можно, например, удалить слишком короткий штрих)
    void finish();

    // Доступ
    const std::vector<Vec2>& pointsWorld() const { return points_; }
    bool empty() const { return points_.size() < 2; }

    // Текущая ширина в мире, и экранная ширина для данного масштаба
    double widthWorld() const { return widthWorld_; }
    double widthScreen(double scale) const { return widthWorld_ * scale; }

private:
    double widthWorld_{1.0};     // фиксируется на begin(): brushPx / cam.scale()
    std::vector<Vec2> points_;   // точки в мировых координатах
};
