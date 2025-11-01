#pragma once
#include <vector>
#include <cstddef>
#include <cstdint>
#include "../types.hpp"

class Camera;

class Stroke {
public:
    Stroke() = default;

    void begin(double brushPx, std::uint32_t colorRGB, const Camera& cam);
    void addScreenPoint(double sx, double sy, const Camera& cam, double minStepPx = 1.5);
    void finish();

    const std::vector<Vec2>& pointsWorld() const { return points_; }
    void translate(const Vec2& delta);
    bool empty() const { return points_.size() < 2; }

    double widthScreen(double currentZoomExp) const;
    std::uint32_t colorRGB() const { return colorRGB_; }

private:
    double widthExp_{0.0}; // log2(width_world)
    std::vector<Vec2> points_;
    std::uint32_t colorRGB_{0xFFFFFF};
};
