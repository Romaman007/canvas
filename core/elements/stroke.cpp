#include "stroke.hpp"
#include "../camera.hpp"   // Camera оперирует Vec2, без Qt
#include <cmath>

static inline double hypot2(double dx, double dy){ return std::sqrt(dx*dx + dy*dy); }

void Stroke::begin(double brushPx, const Camera& cam) {
    widthWorld_ = brushPx / std::max(cam.scale(), 1e-12);
    points_.clear();
}

void Stroke::addScreenPoint(double sx, double sy, const Camera& cam, double minStepPx) {
    // Конвертируем экран → мир
    Vec2 w = cam.worldFromScreen(sx, sy);

    if (points_.empty()) {
        points_.push_back(w);
        return;
    }

    // Дискретизация по экрану: прошлую world-точку проектируем в экран
    const Vec2& lastW = points_.back();
    Vec2 lastS = cam.screenFromWorld(lastW.x, lastW.y);
    double dx = sx - lastS.x;
    double dy = sy - lastS.y;
    if (hypot2(dx, dy) >= minStepPx) {
        points_.push_back(w);
    }
}

void Stroke::finish() {
    // Можно фильтровать артефакты; пока — ничего
    if (points_.size() < 2) points_.clear();
}
