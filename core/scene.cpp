#include "scene.hpp"
#include "camera.hpp"

void Scene::beginStroke(double brushPx, const Camera& cam) {
    if (drawing_) return;
    strokes_.emplace_back();
    strokes_.back().begin(brushPx, cam);
    drawing_ = true;
}

void Scene::addScreenPoint(double sx, double sy, const Camera& cam) {
    if (!drawing_ || strokes_.empty()) return;
    strokes_.back().addScreenPoint(sx, sy, cam, /*minStepPx=*/1.5);
}

void Scene::endStroke() {
    if (!drawing_ || strokes_.empty()) return;
    strokes_.back().finish();
    if (strokes_.back().empty()) {
        strokes_.pop_back();
    }
    drawing_ = false;
}
