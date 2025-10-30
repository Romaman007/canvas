#pragma once
#include <vector>
#include "elements/stroke.hpp"

class Camera;

class Scene {
public:
    Scene() = default;

    void beginStroke(double brushPx, const Camera& cam);
    void addScreenPoint(double sx, double sy, const Camera& cam);
    void endStroke();

    const std::vector<Stroke>& strokes() const { return strokes_; }

private:
    std::vector<Stroke> strokes_;
    bool drawing_ = false;
};
