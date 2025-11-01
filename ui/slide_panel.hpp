#pragma once
#include <QColor>
#include <QFrame>
#include <QHash>
#include <QString>
#include "tool_mode.hpp"

class QButtonGroup;
class QDoubleSpinBox;
class QLineEdit;
class QStackedWidget;

namespace ui {

class SlidePanel : public QFrame {
    Q_OBJECT
public:
    explicit SlidePanel(QWidget* parent = nullptr);

    void setMode(Mode mode);
    void setBrushWidth(double px);
    void setBrushColor(const QColor& color);
    int preferredWidth() const { return width(); }

signals:
    void modeRequested(Mode mode);
    void brushWidthChanged(double px);
    void brushColorChanged(const QColor& color);

private:
    void setupUi();
    void handleModeButton(int id);
    void handleColorButton(int id);
    void applyColorToButtons(const QColor& color);
    QColor colorFromHex(const QString& hex) const;
    void setColorEditText(const QColor& color);
    void emitColor(const QColor& color);

private:
    QButtonGroup* modeButtons_{nullptr};
    QStackedWidget* modeStack_{nullptr};
    QDoubleSpinBox* brushSpin_{nullptr};
    QButtonGroup* colorButtons_{nullptr};
    QLineEdit* colorEdit_{nullptr};
    QHash<Mode, int> pageMap_;
    QColor currentColor_{Qt::white};
};

} // namespace ui

