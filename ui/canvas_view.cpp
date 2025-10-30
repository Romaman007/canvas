#include "canvas_view.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFontMetrics>
#include <cmath>
#include <algorithm>

CanvasView::CanvasView(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);
}

void CanvasView::resizeEvent(QResizeEvent*) {
    cam_.setOffsetPx(width() / 2.0, height() / 2.0);
    update();
}

void CanvasView::wheelEvent(QWheelEvent* e) {
    const double angle = e->angleDelta().y() / 120.0;
    const double deltaExp = angle / 6.0;
    cam_.zoomAt(e->position().x(), e->position().y(), deltaExp);
    update();
}

void CanvasView::mousePressEvent(QMouseEvent* e) {
    if (mode_ == Mode::Navigate) {
        if (e->button() == Qt::MiddleButton ||
            (e->button() == Qt::LeftButton && spaceDown_)) {
            panning_ = true;
            lastPos_ = e->position();
            setCursor(Qt::ClosedHandCursor);
        }
    } else if (mode_ == Mode::Draw) {
        if (e->button() == Qt::LeftButton) {
            scene_.beginStroke(brushPx_, cam_);
            scene_.addScreenPoint(e->position().x(), e->position().y(), cam_);
            update();
        }
    }
}

void CanvasView::mouseMoveEvent(QMouseEvent* e) {
    if (mode_ == Mode::Navigate) {
        if (!panning_) return;
        QPointF d = e->position() - lastPos_;
        lastPos_ = e->position();
        cam_.panPx(d.x(), d.y());
        update();
    } else if (mode_ == Mode::Draw) {
        if (e->buttons() & Qt::LeftButton) {
            scene_.addScreenPoint(e->position().x(), e->position().y(), cam_);
            update();
        }
    }
}

void CanvasView::mouseReleaseEvent(QMouseEvent* e) {
    if (mode_ == Mode::Navigate) {
        if (panning_ && (e->button() == Qt::MiddleButton || e->button() == Qt::LeftButton)) {
            panning_ = false;
            setCursor(Qt::ArrowCursor);
        }
    } else if (mode_ == Mode::Draw) {
        if (e->button() == Qt::LeftButton) {
            scene_.endStroke();
            update();
        }
    }
}

void CanvasView::keyPressEvent(QKeyEvent* e) {
    switch (e->key()) {
    case Qt::Key_Space:
        spaceDown_ = true;
        if (mode_ == Mode::Navigate && underMouse()) setCursor(Qt::OpenHandCursor);
        break;
    case Qt::Key_D:
        mode_ = Mode::Draw;
        setCursor(Qt::CrossCursor);
        update();
        break;
    case Qt::Key_N:
        mode_ = Mode::Navigate;
        setCursor(Qt::ArrowCursor);
        update();
        break;
    case Qt::Key_BracketLeft:
        brushPx_ = std::max(0.5, brushPx_ - 1.0);
        update();
        break;
    case Qt::Key_BracketRight:
        brushPx_ = std::min(256.0, brushPx_ + 1.0);
        update();
        break;
    default:
        break;
    }
    QWidget::keyPressEvent(e);
}

void CanvasView::keyReleaseEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Space) {
        spaceDown_ = false;
        if (!panning_ && mode_ == Mode::Navigate) setCursor(Qt::ArrowCursor);
    }
    QWidget::keyReleaseEvent(e);
}

void CanvasView::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor(24, 26, 27));

    drawGrid(p);
    drawStrokes(p);
    drawHud(p);
}

void CanvasView::drawGrid(QPainter& p) {
    const double sc = cam_.scale();
    if (sc < 1e-9 || sc > 1e10) return;

    const double targetPx = 48.0;
    double worldStep = targetPx / std::max(sc, 1e-12);

    const double exp10 = std::floor(std::log10(std::max(worldStep, 1e-12)));
    const double base = std::pow(10.0, exp10);
    const double candidates[] = {1, 2, 5, 10};
    for (double m : candidates) {
        if (m * base >= worldStep) { worldStep = m * base; break; }
    }

    const Vec2 tl = cam_.worldFromScreen(0, 0);
    const Vec2 br = cam_.worldFromScreen(width(), height());

    const double x0 = std::floor(tl.x / worldStep) * worldStep;
    const double y0 = std::floor(tl.y / worldStep) * worldStep;

    const int maxLines = 500;
    int lineCountX = (int)((br.x - tl.x) / worldStep);
    int lineCountY = (int)((br.y - tl.y) / worldStep);
    if (lineCountX > maxLines) worldStep *= std::ceil(lineCountX / (double)maxLines);
    if (lineCountY > maxLines) worldStep *= std::ceil(lineCountY / (double)maxLines);

    QPen minor(QColor(60, 62, 64));  minor.setWidthF(1.0);
    QPen major(QColor(80, 84, 88));  major.setWidthF(1.2);

    int i = 0;
    for (double x = x0; x < br.x + worldStep; x += worldStep, ++i) {
        const Vec2 sx = cam_.screenFromWorld(x, 0);
        p.setPen((i % 5 == 0) ? major : minor);
        p.drawLine(QPointF(sx.x, 0), QPointF(sx.x, height()));
    }

    int j = 0;
    for (double y = y0; y < br.y + worldStep; y += worldStep, ++j) {
        const Vec2 sy = cam_.screenFromWorld(0, y);
        p.setPen((j % 5 == 0) ? major : minor);
        p.drawLine(QPointF(0, sy.y), QPointF(width(), sy.y));
    }
}

void CanvasView::drawStrokes(QPainter& p) {
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setBrush(Qt::NoBrush);

    for (const auto& s : scene_.strokes()) {
        const auto& pts = s.pointsWorld();
        if (pts.size() < 2) continue;

        const double penPx = s.widthScreen(cam_.scale());
        QPen pen(QColor(230, 230, 230));
        pen.setWidthF(penPx);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        p.setPen(pen);

        QPainterPath path;
        const Vec2 w0 = pts[0];
        const Vec2 s0 = cam_.screenFromWorld(w0.x, w0.y);
        path.moveTo(s0.x, s0.y);
        for (size_t i = 1; i < pts.size(); ++i) {
            const Vec2 wi = pts[i];
            const Vec2 si = cam_.screenFromWorld(wi.x, wi.y);
            path.lineTo(si.x, si.y);
        }
        p.drawPath(path);
    }
}

void CanvasView::drawHud(QPainter& p) {
    const double sc = cam_.scale();
    const QString scaleText = QString("Scale: %1×").arg(sc, 0, 'f', 2);
    const QString brushText = QString("Brush: %1 px").arg(brushPx_, 0, 'f', 1);
    const QString modeText  = (mode_ == Mode::Draw)
        ? "Mode: Draw (D)  |  N = Navigate"
        : "Mode: Navigate (N)  |  D = Draw";
    const QString helpText  = "Pan: MMB or Space+LMB  |  Zoom: Wheel  |  Brush: [ / ]";

    const QString text = scaleText + "    " + brushText + "    " + modeText + "    " + helpText;

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
