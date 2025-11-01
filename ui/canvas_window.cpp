#include "canvas_window.hpp"

#include <QEasingCurve>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QToolButton>
#include <QVariantAnimation>
#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>
#include <cmath>

#include "canvas_view.hpp"
#include "slide_panel.hpp"

namespace {
constexpr int kAnimationDurationMs = 220;
constexpr double kEpsilon = 1e-4;
}

CanvasWindow::CanvasWindow(QWidget* parent)
    : QMainWindow(parent) {
    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    view_ = new CanvasView(&scene_, central);
    layout->addWidget(view_);
    setCentralWidget(central);

    panelContainer_ = new QWidget(central);
    panelContainer_->setObjectName(QStringLiteral("PanelContainer"));
    panelContainer_->setStyleSheet(QStringLiteral(
        "#PanelContainer {"
        "  background: rgba(20, 22, 27, 0.92);"
        "  border: 1px solid rgba(70, 76, 96, 0.6);"
        "  border-top-right-radius: 14px;"
        "  border-bottom-right-radius: 14px;"
        "}"
        "#PanelHandle {"
        "  background: rgba(20, 22, 27, 0.92);"
        "  border-top-right-radius: 14px;"
        "  border-bottom-right-radius: 14px;"
        "}"
    ));

    auto* containerLayout = new QHBoxLayout(panelContainer_);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(overlaySpacing_);

    panel_ = new ui::SlidePanel(panelContainer_);
    panel_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    containerLayout->addWidget(panel_);

    handleWidget_ = new QWidget(panelContainer_);
    handleWidget_->setObjectName(QStringLiteral("PanelHandle"));
    handleWidget_->setFixedWidth(handleWidth_);
    handleWidget_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    auto* handleLayout = new QVBoxLayout(handleWidget_);
    handleLayout->setContentsMargins(0, 0, 0, 0);
    handleLayout->setSpacing(0);

    toggleButton_ = new QToolButton(handleWidget_);
    toggleButton_->setText(QStringLiteral("\u25B6"));
    toggleButton_->setCheckable(true);
    toggleButton_->setAutoRaise(true);
    toggleButton_->setCursor(Qt::PointingHandCursor);
    toggleButton_->setToolTip(tr("Tool panel"));
    toggleButton_->setStyleSheet(QStringLiteral(
        "QToolButton {"
        "  color: #f4f6f8;"
        "  background: rgba(28, 30, 34, 0.88);"
        "  border-radius: 10px;"
        "  border: 1px solid rgba(86, 93, 108, 0.55);"
        "  font-size: 18px;"
        "  padding: 4px;"
        "}"
        "QToolButton:hover {"
        "  background: rgba(120, 140, 255, 0.30);"
        "  border-color: rgba(120, 140, 255, 0.7);"
        "}"
        "QToolButton:checked {"
        "  background: rgba(120, 140, 255, 0.40);"
        "  border-color: rgba(120, 140, 255, 0.85);"
        "}"
    ));
    toggleButton_->setFixedSize(toggleSize_);
    toggleButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    handleLayout->addWidget(toggleButton_, 0, Qt::AlignVCenter | Qt::AlignHCenter);
    handleLayout->addStretch(1);

    containerLayout->addWidget(handleWidget_, 0, Qt::AlignRight);

    panelFullWidth_ = std::max(panel_->sizeHint().width(), 260);
    panel_->setMinimumWidth(0);
    panel_->setMaximumWidth(panelFullWidth_);
    panel_->hide();
    panelContainer_->setFixedWidth(handleWidth_);
    panelContainer_->show();
    panelContainer_->raise();

    panelAnimation_ = new QVariantAnimation(this);
    panelAnimation_->setDuration(kAnimationDurationMs);
    panelAnimation_->setEasingCurve(QEasingCurve::InOutQuad);
    connect(panelAnimation_, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
        panelProgress_ = value.toDouble();
        applyOverlayGeometry();
    });
    connect(panelAnimation_, &QVariantAnimation::finished, this, [this]() {
        panelProgress_ = panelExpanded_ ? 1.0 : 0.0;
        if (!panelExpanded_) {
            panel_->hide();
        }
        applyOverlayGeometry();
    });

    connect(toggleButton_, &QToolButton::clicked, this, &CanvasWindow::handlePanelToggle);
    connect(panel_, &ui::SlidePanel::modeRequested, this, &CanvasWindow::handleModeRequested);
    connect(panel_, &ui::SlidePanel::brushWidthChanged, this, &CanvasWindow::handleBrushWidthRequested);
    connect(panel_, &ui::SlidePanel::brushColorChanged, this, &CanvasWindow::handleBrushColorRequested);
    connect(view_, &CanvasView::modeChanged, this, &CanvasWindow::handleViewModeChanged);
    connect(view_, &CanvasView::brushWidthChanged, panel_, &ui::SlidePanel::setBrushWidth);
    connect(view_, &CanvasView::brushColorChanged, panel_, &ui::SlidePanel::setBrushColor);

    panel_->setBrushWidth(view_->brushWidth());
    panel_->setBrushColor(view_->brushColor());
    panel_->setMode(view_->mode());

    panelExpanded_ = false;
    panelProgress_ = 0.0;
    toggleButton_->setChecked(false);

    updateOverlayLayout();
    applyOverlayGeometry();
}

void CanvasWindow::resizeEvent(QResizeEvent* e) {
    QMainWindow::resizeEvent(e);
    updateOverlayLayout();
}

void CanvasWindow::updateOverlayLayout() {
    if (!centralWidget() || !panelContainer_ || !panel_ || !handleWidget_ || !toggleButton_) return;

    const QRect area = centralWidget()->rect();
    const int margin = overlayMargin_;
    const int availableHeight = std::max(area.height() - margin * 2, toggleSize_.height());

    panelFullWidth_ = std::max(panelFullWidth_, panel_->sizeHint().width());

    panelContainer_->setFixedHeight(availableHeight);
    handleWidget_->setFixedHeight(availableHeight);
    panel_->setFixedHeight(availableHeight);

    if (!panelAnimation_ || panelAnimation_->state() != QVariantAnimation::Running) {
        panelProgress_ = panelExpanded_ ? 1.0 : 0.0;
    }

    applyOverlayGeometry();
}

void CanvasWindow::applyOverlayGeometry() {
    if (!panelContainer_ || !panel_ || !handleWidget_ || !toggleButton_) return;

    const int margin = overlayMargin_;
    const double t = std::clamp(panelProgress_, 0.0, 1.0);
    const int panelWidth = static_cast<int>(std::round(panelFullWidth_ * t));

    panel_->setVisible(panelWidth > 0);
    panel_->setMinimumWidth(panelWidth);
    panel_->setMaximumWidth(panelWidth);

    const int containerWidth = handleWidth_ + panelWidth;
    panelContainer_->setFixedWidth(containerWidth);
    panelContainer_->move(margin, margin);
    panelContainer_->setVisible(true);

    const QString arrow = (t > 0.5) ? QStringLiteral("\u25C0") : QStringLiteral("\u25B6");
    if (toggleButton_->text() != arrow) {
        toggleButton_->setText(arrow);
    }

    panelContainer_->raise();
    panel_->raise();
    handleWidget_->raise();
    toggleButton_->raise();
}

void CanvasWindow::setPanelExpanded(bool expanded) {
    if (!panel_ || !panelAnimation_) return;

    const double target = expanded ? 1.0 : 0.0;
    const bool animationRunning = panelAnimation_->state() == QVariantAnimation::Running;

    if (!animationRunning && std::abs(panelProgress_ - target) < kEpsilon && panelExpanded_ == expanded) {
        panelProgress_ = target;
        panel_->setVisible(expanded);
        toggleButton_->setChecked(expanded);
        applyOverlayGeometry();
        return;
    }

    panelExpanded_ = expanded;
    toggleButton_->setChecked(expanded);

    const double start = animationRunning ? panelProgress_ : panelProgress_;

    panelAnimation_->stop();
    panelProgress_ = start;
    applyOverlayGeometry();
    panel_->setVisible(true);
    panelAnimation_->setStartValue(start);
    panelAnimation_->setEndValue(target);
    panelAnimation_->start();
}

void CanvasWindow::handlePanelToggle(bool checked) {
    setPanelExpanded(checked);
}

void CanvasWindow::handleViewModeChanged(ui::Mode mode) {
    panel_->setMode(mode);
    if (mode == ui::Mode::Draw) {
        setPanelExpanded(true);
    }
}

void CanvasWindow::handleModeRequested(ui::Mode mode) {
    view_->setMode(mode);
    if (mode == ui::Mode::Draw) {
        setPanelExpanded(true);
    }
}

void CanvasWindow::handleBrushColorRequested(const QColor& color) {
    view_->setBrushColor(color);
}

void CanvasWindow::handleBrushWidthRequested(double px) {
    view_->setBrushWidth(px);
}
