#pragma once
#include <QColor>
#include <QMainWindow>
#include <QSize>
#include "../core/scene.hpp"
#include "tool_mode.hpp"

class QToolButton;
class QVariantAnimation;
class QWidget;

class CanvasView;

namespace ui {
class SlidePanel;
}

class CanvasWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit CanvasWindow(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent*) override;

private:
    void updateOverlayLayout();
    void applyOverlayGeometry();
    void setPanelExpanded(bool expanded);
    void handlePanelToggle(bool checked);
    void handleViewModeChanged(ui::Mode mode);
    void handleModeRequested(ui::Mode mode);
    void handleBrushWidthRequested(double px);
    void handleBrushColorRequested(const QColor& color);

private:
    Scene scene_;
    CanvasView* view_{nullptr};
    QWidget* panelContainer_{nullptr};
    QWidget* handleWidget_{nullptr};
    ui::SlidePanel* panel_{nullptr};
    QToolButton* toggleButton_{nullptr};
    QVariantAnimation* panelAnimation_{nullptr};
    bool panelExpanded_ = false;
    double panelProgress_ = 0.0;
    int overlayMargin_ = 0;
    int overlaySpacing_ = 0;
    int handleWidth_ = 44;
    int panelFullWidth_ = 280;
    QSize toggleSize_{32, 32};
};
