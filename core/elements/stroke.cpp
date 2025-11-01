#include "stroke.hpp"
#include "../camera.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace {
constexpr double kMinScaleExp = -1022.0;
constexpr double kMaxScaleExp = 1023.0;
constexpr double kMinWorldExp = -60.0;
constexpr double kMaxWorldExp = 60.0;
constexpr double kMinScreenExp = -24.0;
constexpr double kMaxScreenExp = 12.0;
constexpr double kMinBrushPx = 1e-12;
}

static inline double hypot2(double dx, double dy){ return std::sqrt(dx*dx + dy*dy); }

void Stroke::begin(double brushPx, std::uint32_t colorRGB, const Camera& cam) {
    const double safePx = std::max(brushPx, kMinBrushPx);
    widthExp_ = std::clamp(std::log2(safePx) - cam.zoomExp(), kMinWorldExp, kMaxWorldExp);
    points_.clear();
    colorRGB_ = colorRGB;
}

void Stroke::addScreenPoint(double sx, double sy, const Camera& cam, double minStepPx) {
    Vec2 w = cam.worldFromScreen(sx, sy);

    if (points_.empty()) {
        points_.push_back(w);
        return;
    }

    const Vec2& lastW = points_.back();
    Vec2 lastS = cam.screenFromWorld(lastW.x, lastW.y);
    double dx = sx - lastS.x;
    double dy = sy - lastS.y;
    if (hypot2(dx, dy) >= minStepPx) {
        points_.push_back(w);
    }
}

void Stroke::finish() {
    if (points_.size() < 2) points_.clear();
}

void Stroke::translate(const Vec2& delta) {
    if (delta.x == 0.0 && delta.y == 0.0) return;
    for (auto& pt : points_) {
        pt.x -= delta.x;
        pt.y -= delta.y;
    }
}

double Stroke::widthScreen(double currentZoomExp) const {
    double expSum = widthExp_ + currentZoomExp;
    if (expSum < kMinScaleExp) return 0.0;
    expSum = std::clamp(expSum, kMinScreenExp, kMaxScreenExp);
    return std::exp2(expSum);
}
