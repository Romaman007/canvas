#pragma once
#include <vector>
#include <cstdint>
#include "elements/stroke.hpp"

class Camera;

class Scene {
public:
    Scene() = default;

    void beginStroke(double brushPx, std::uint32_t colorRGB, const Camera& cam);
    void addScreenPoint(double sx, double sy, const Camera& cam);
    void endStroke();
    void translate(const Vec2& delta);

    const std::vector<Stroke>& strokes() const { return strokes_; }

private:
    std::vector<Stroke> strokes_;
    bool drawing_ = false;
};
