#include "canvas_view.hpp"

#include <QColor>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>
#include <QtGlobal>
#include <algorithm>
#include <cmath>
#include <limits>

#include "../core/scene.hpp"

namespace {
std::uint32_t rgbFromQColor(const QColor& color) {
    return (static_cast<std::uint32_t>(color.red()) << 16) |
           (static_cast<std::uint32_t>(color.green()) << 8) |
            static_cast<std::uint32_t>(color.blue());
}

QColor colorFromRgb(std::uint32_t rgb) {
    return QColor(
        static_cast<int>((rgb >> 16) & 0xFF),
        static_cast<int>((rgb >> 8) & 0xFF),
        static_cast<int>(rgb & 0xFF)
    );
}
}

CanvasView::CanvasView(Scene* scene, QWidget* parent)
    : QWidget(parent)
    , scene_(scene) {
    Q_ASSERT(scene_);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);
}

void CanvasView::setMode(ui::Mode mode) {
    if (mode_ == mode) return;
    mode_ = mode;

    switch (mode_) {
    case ui::Mode::Pan:
        if (spaceDown_) {
            setCursor(Qt::OpenHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
        break;
    case ui::Mode::Draw:
        setCursor(Qt::CrossCursor);
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }

    emit modeChanged(mode_);
    update();
}

void CanvasView::setBrushWidth(double px) {
    px = std::clamp(px, 0.5, 256.0);
    if (std::abs(brushPx_ - px) < 1e-6) return;
    brushPx_ = px;
    emit brushWidthChanged(brushPx_);
    update();
}

void CanvasView::setBrushColor(const QColor& color) {
    if (!color.isValid()) return;
    const std::uint32_t rgb = rgbFromQColor(color);
    if (brushColorRGB_ == rgb) return;
    brushColorRGB_ = rgb;
    emit brushColorChanged(colorFromRgb(brushColorRGB_));
    update();
}

QColor CanvasView::brushColor() const {
    return colorFromRgb(brushColorRGB_);
}

void CanvasView::resizeEvent(QResizeEvent*) {
    cam_.setOffsetPx(width() / 2.0, height() / 2.0);
    update();
}

void CanvasView::wheelEvent(QWheelEvent* e) {
    const double angle = e->angleDelta().y() / 120.0;
    const double deltaExp = angle / 6.0;
    cam_.zoomAt(e->position().x(), e->position().y(), deltaExp);
    recenterSceneIfNeeded();
    update();
}

void CanvasView::mousePressEvent(QMouseEvent* e) {
    if (!scene_) return;

    if (mode_ == ui::Mode::Pan) {
        if (e->button() == Qt::MiddleButton ||
            (e->button() == Qt::LeftButton && spaceDown_)) {
            panning_ = true;
            lastPos_ = e->position();
            setCursor(Qt::ClosedHandCursor);
        }
    } else if (mode_ == ui::Mode::Draw) {
        if (e->button() == Qt::LeftButton) {
            scene_->beginStroke(brushPx_, brushColorRGB_, cam_);
            scene_->addScreenPoint(e->position().x(), e->position().y(), cam_);
            update();
        }
    }
}

void CanvasView::mouseMoveEvent(QMouseEvent* e) {
    if (!scene_) return;

    if (mode_ == ui::Mode::Pan) {
        if (!panning_) return;
        QPointF d = e->position() - lastPos_;
        lastPos_ = e->position();
        cam_.panPx(d.x(), d.y());
        update();
    } else if (mode_ == ui::Mode::Draw) {
        if (e->buttons() & Qt::LeftButton) {
            scene_->addScreenPoint(e->position().x(), e->position().y(), cam_);
            update();
        }
    }
}

void CanvasView::mouseReleaseEvent(QMouseEvent* e) {
    if (!scene_) return;

    if (mode_ == ui::Mode::Pan) {
        if (panning_ && (e->button() == Qt::MiddleButton || e->button() == Qt::LeftButton)) {
            panning_ = false;
            setCursor(Qt::ArrowCursor);
        }
    } else if (mode_ == ui::Mode::Draw) {
        if (e->button() == Qt::LeftButton) {
            scene_->endStroke();
            update();
        }
    }
}

void CanvasView::keyPressEvent(QKeyEvent* e) {
    switch (e->key()) {
    case Qt::Key_Space:
        spaceDown_ = true;
        if (mode_ == ui::Mode::Pan && underMouse()) setCursor(Qt::OpenHandCursor);
        break;
    case Qt::Key_D:
        setMode(ui::Mode::Draw);
        break;
    case Qt::Key_N:
        setMode(ui::Mode::Pan);
        break;
    case Qt::Key_BracketLeft:
        setBrushWidth(brushPx_ - 1.0);
        break;
    case Qt::Key_BracketRight:
        setBrushWidth(brushPx_ + 1.0);
        break;
    default:
        break;
    }
    QWidget::keyPressEvent(e);
}

void CanvasView::keyReleaseEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Space) {
        spaceDown_ = false;
        if (!panning_ && mode_ == ui::Mode::Pan) setCursor(Qt::ArrowCursor);
    }
    QWidget::keyReleaseEvent(e);
}

void CanvasView::paintEvent(QPaintEvent*) {
    recenterSceneIfNeeded();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor(24, 26, 27));

    drawGrid(p);
    drawStrokes(p);
    drawHud(p);
}

void CanvasView::drawGrid(QPainter& p) {
    const double targetPx = 48.0;
    const double sc = cam_.scale();
    if (!std::isfinite(sc) || sc <= 0.0) return;

    double worldStep = targetPx / sc;
    if (!std::isfinite(worldStep) || worldStep <= 0.0) return;

    const double exp10 = std::floor(std::log10(std::max(worldStep, std::numeric_limits<double>::min())));
    const double base = std::pow(10.0, exp10);
    const double candidates[] = {1.0, 2.0, 5.0, 10.0};
    for (double m : candidates) {
        if (m * base >= worldStep) { worldStep = m * base; break; }
    }

    const Vec2 tl = cam_.worldFromScreen(0, 0);
    const Vec2 br = cam_.worldFromScreen(width(), height());

    if (!std::isfinite(tl.x) || !std::isfinite(tl.y) ||
        !std::isfinite(br.x) || !std::isfinite(br.y)) {
        return;
    }

    double worldLeft = std::min(tl.x, br.x);
    double worldRight = std::max(tl.x, br.x);
    double worldTop = std::min(tl.y, br.y);
    double worldBottom = std::max(tl.y, br.y);
    const int maxLines = 500;

    QPen minor(QColor(60, 62, 64));  minor.setWidthF(1.0);
    QPen major(QColor(80, 84, 88));  major.setWidthF(1.2);

    int i = 0;
    const int limit = maxLines + 2;
    for (double x = std::floor(worldLeft / worldStep) * worldStep; i < limit && x <= worldRight + worldStep; x += worldStep, ++i) {
        const Vec2 sx = cam_.screenFromWorld(x, 0);
        if (!std::isfinite(sx.x)) break;
        p.setPen((i % 5 == 0) ? major : minor);
        p.drawLine(QPointF(sx.x, 0), QPointF(sx.x, height()));
    }

    int j = 0;
    for (double y = std::floor(worldTop / worldStep) * worldStep; j < limit && y <= worldBottom + worldStep; y += worldStep, ++j) {
        const Vec2 sy = cam_.screenFromWorld(0, y);
        if (!std::isfinite(sy.y)) break;
        p.setPen((j % 5 == 0) ? major : minor);
        p.drawLine(QPointF(0, sy.y), QPointF(width(), sy.y));
    }
}

void CanvasView::drawStrokes(QPainter& p) {
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setBrush(Qt::NoBrush);

    if (!scene_) return;

    for (const auto& s : scene_->strokes()) {
        const auto& pts = s.pointsWorld();
        if (pts.size() < 2) continue;

        double penPx = s.widthScreen(cam_.zoomExp());
        if (penPx < 0.05) continue;
        if (penPx > 4096.0) penPx = 4096.0;

        QPen pen(colorFromRgb(s.colorRGB()));
        pen.setWidthF(penPx);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        p.setPen(pen);

        QPainterPath path;
        Vec2 first = cam_.screenFromWorld(pts[0].x, pts[0].y);
        path.moveTo(first.x, first.y);
        for (size_t i = 1; i < pts.size(); ++i) {
            Vec2 screenPt = cam_.screenFromWorld(pts[i].x, pts[i].y);
            path.lineTo(screenPt.x, screenPt.y);
        }
        p.drawPath(path);
    }
}

void CanvasView::drawHud(QPainter& p) {
    const double sc = cam_.scale();
    const double exp = std::log2(std::max(sc, 1e-12));
    const QString text = QStringLiteral("Scale: 2^%1").arg(exp, 0, 'f', 2);

    QFont f = p.font();
    f.setPointSizeF(f.pointSizeF() + 1);
    p.setFont(f);
    const QFontMetrics fm(p.font());

    const int pad = 8;
    const int textW = fm.horizontalAdvance(text);
    const int textH = fm.height();

    QRect r(rect().right() - textW - pad * 2 - 12,
            rect().bottom() - textH - pad * 2 - 12,
            textW + pad * 2,
            textH + pad * 2);

    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 120));
    p.drawRoundedRect(r, 6, 6);

    p.setPen(QColor(235, 235, 235));
    p.drawText(r.adjusted(pad, pad, -pad, -pad),
               Qt::AlignLeft | Qt::AlignVCenter, text);
}

void CanvasView::recenterSceneIfNeeded() {
    if (!scene_) return;
    if (!cam_.needsRecenter()) return;
    Vec2 delta = cam_.worldCenter();
    if (delta.x == 0.0 && delta.y == 0.0) return;
    scene_->translate(delta);
    cam_.shiftWorldCenter(delta);
}
