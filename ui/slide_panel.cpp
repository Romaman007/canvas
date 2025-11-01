#include "slide_panel.hpp"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QStringList>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>

namespace {
constexpr double kMinBrushPx = 0.5;
constexpr double kMaxBrushPx = 256.0;
constexpr double kBrushStep  = 1.0;

const QStringList kColorPalette = {
    QStringLiteral("FFFFFF"),
    QStringLiteral("E6E6E6"),
    QStringLiteral("C9C9C9"),
    QStringLiteral("9E9E9E"),
    QStringLiteral("5F5F5F"),
    QStringLiteral("262626"),
    QStringLiteral("FF5C57"),
    QStringLiteral("FFBD2E"),
    QStringLiteral("FEFF51"),
    QStringLiteral("5AF78D"),
    QStringLiteral("57C7FF"),
    QStringLiteral("BD93F9"),
    QStringLiteral("FF79C6"),
    QStringLiteral("FFA500"),
    QStringLiteral("00C2A0"),
    QStringLiteral("0091EA")
};

QString hexFromColor(const QColor& color) {
    return QStringLiteral("%1%2%3")
        .arg(color.red(),   2, 16, QLatin1Char('0'))
        .arg(color.green(), 2, 16, QLatin1Char('0'))
        .arg(color.blue(),  2, 16, QLatin1Char('0'))
        .toUpper();
}
} // namespace

namespace ui {

SlidePanel::SlidePanel(QWidget* parent)
    : QFrame(parent) {
    setObjectName(QStringLiteral("SlidePanel"));
    setFrameShape(QFrame::StyledPanel);
    setAutoFillBackground(true);
    currentColor_ = QColor(QStringLiteral("#E6E6E6"));
    setStyleSheet(QStringLiteral(
        "#SlidePanel {"
        "  background: rgba(28, 30, 34, 0.96);"
        "  color: #f4f6f8;"
        "  border-top-right-radius: 14px;"
        "  border-bottom-right-radius: 14px;"
        "  border: 1px solid rgba(86, 93, 108, 0.55);"
        "}"
        "#SlidePanel QLabel {"
        "  color: #c7cbd6;"
        "  font-size: 12px;"
        "  letter-spacing: 0.4px;"
        "}"
        "#SlidePanel QToolButton {"
        "  color: #f4f6f8;"
        "  background: rgba(255, 255, 255, 0.04);"
        "  border: 1px solid rgba(86, 93, 108, 0.35);"
        "  border-radius: 8px;"
        "  padding: 6px 12px;"
        "  text-align: left;"
        "}"
        "#SlidePanel QToolButton:hover {"
        "  background: rgba(255, 255, 255, 0.08);"
        "  border-color: rgba(120, 140, 255, 0.45);"
        "}"
        "#SlidePanel QToolButton:checked {"
        "  background: rgba(120, 140, 255, 0.32);"
        "  border-color: rgba(120, 140, 255, 0.65);"
        "}"
        "#SlidePanel QDoubleSpinBox {"
        "  background: rgba(16, 18, 22, 0.9);"
        "  border: 1px solid rgba(120, 140, 255, 0.45);"
        "  border-radius: 8px;"
        "  padding: 4px 8px;"
        "  color: #f8f9fb;"
        "  selection-background-color: rgba(120, 140, 255, 0.65);"
        "}"
        "#SlidePanel QDoubleSpinBox::up-button,"
        "#SlidePanel QDoubleSpinBox::down-button {"
        "  width: 16px;"
        "  background: transparent;"
        "  border: none;"
        "}"
        "#SlidePanel QLineEdit {"
        "  background: rgba(16, 18, 22, 0.9);"
        "  border: 1px solid rgba(120, 140, 255, 0.45);"
        "  border-radius: 8px;"
        "  padding: 6px 10px;"
        "  color: #f8f9fb;"
        "  selection-background-color: rgba(120, 140, 255, 0.65);"
        "  font-family: \"Fira Mono\", \"Consolas\", monospace;"
        "  letter-spacing: 1.2px;"
        "}"
    ));
    setupUi();
    const int targetWidth = std::max(260, sizeHint().width());
    setMinimumWidth(targetWidth);
    setMaximumWidth(targetWidth);
    setFixedWidth(targetWidth);
    applyColorToButtons(currentColor_);
    setColorEditText(currentColor_);
}

void SlidePanel::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    auto* title = new QLabel(tr("Tools"), this);
    title->setStyleSheet(QStringLiteral("font-weight: 600;"));
    layout->addWidget(title);

    auto* modesLabel = new QLabel(tr("Modes"), this);
    modesLabel->setStyleSheet(QStringLiteral("color: #cccccc;"));
    layout->addWidget(modesLabel);

    modeButtons_ = new QButtonGroup(this);
    modeButtons_->setExclusive(true);
    connect(modeButtons_, &QButtonGroup::idToggled, this, [this](int id, bool checked) {
        if (checked) handleModeButton(id);
    });

    auto* navButton = new QToolButton(this);
    navButton->setText(tr("Navigate"));
    navButton->setCheckable(true);
    navButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    navButton->setArrowType(Qt::NoArrow);
    navButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    modeButtons_->addButton(navButton, static_cast<int>(Mode::Pan));
    layout->addWidget(navButton);

    auto* drawButton = new QToolButton(this);
    drawButton->setText(tr("Draw"));
    drawButton->setCheckable(true);
    drawButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    drawButton->setArrowType(Qt::NoArrow);
    drawButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    modeButtons_->addButton(drawButton, static_cast<int>(Mode::Draw));
    layout->addWidget(drawButton);

    modeStack_ = new QStackedWidget(this);
    modeStack_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* navPage = new QWidget(this);
    auto* navLayout = new QVBoxLayout(navPage);
    navLayout->setContentsMargins(0, 0, 0, 0);
    navLayout->setSpacing(8);
    auto* navInfo = new QLabel(tr("Navigation mode has no settings yet."), navPage);
    navInfo->setWordWrap(true);
    navLayout->addWidget(navInfo);
    navLayout->addStretch(1);

    auto* drawPage = new QWidget(this);
    auto* drawLayout = new QVBoxLayout(drawPage);
    drawLayout->setContentsMargins(0, 0, 0, 0);
    drawLayout->setSpacing(10);

    auto* strokeLabel = new QLabel(tr("Stroke Width"), drawPage);
    brushSpin_ = new QDoubleSpinBox(drawPage);
    brushSpin_->setRange(kMinBrushPx, kMaxBrushPx);
    brushSpin_->setSingleStep(kBrushStep);
    brushSpin_->setDecimals(1);
    brushSpin_->setValue(4.0);
    connect(brushSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &SlidePanel::brushWidthChanged);

    drawLayout->addWidget(strokeLabel);
    drawLayout->addWidget(brushSpin_);

    auto* colorLabel = new QLabel(tr("Color"), drawPage);
    colorLabel->setStyleSheet(QStringLiteral("margin-top: 6px;"));
    drawLayout->addWidget(colorLabel);

    colorButtons_ = new QButtonGroup(this);
    colorButtons_->setExclusive(true);
    connect(colorButtons_, &QButtonGroup::idToggled, this, [this](int id, bool checked) {
        if (checked) handleColorButton(id);
    });

    auto* paletteWidget = new QWidget(drawPage);
    auto* paletteLayout = new QGridLayout(paletteWidget);
    paletteLayout->setContentsMargins(0, 0, 0, 0);
    paletteLayout->setHorizontalSpacing(6);
    paletteLayout->setVerticalSpacing(6);

    const int columns = 6;
    int row = 0;
    int col = 0;
    for (int i = 0; i < kColorPalette.size(); ++i) {
        const QString& hex = kColorPalette.at(i);
        auto* btn = new QToolButton(paletteWidget);
        btn->setCheckable(true);
        btn->setAutoRaise(false);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedSize(28, 28);
        btn->setProperty("hex", hex);
        btn->setStyleSheet(QStringLiteral(
            "QToolButton {"
            "  border: 1px solid rgba(90, 96, 110, 0.55);"
            "  border-radius: 6px;"
            "  background: #%1;"
            "}"
            "QToolButton:hover {"
            "  border-color: rgba(255, 255, 255, 0.7);"
            "}"
            "QToolButton:checked {"
            "  border: 2px solid rgba(255, 255, 255, 0.9);"
            "}"
        ).arg(hex));
        colorButtons_->addButton(btn, i);
        paletteLayout->addWidget(btn, row, col);

        ++col;
        if (col >= columns) {
            col = 0;
            ++row;
        }
    }

    drawLayout->addWidget(paletteWidget);

    auto* hexLabel = new QLabel(tr("HEX (RRGGBB, without #)"), drawPage);
    drawLayout->addWidget(hexLabel);

    colorEdit_ = new QLineEdit(drawPage);
    colorEdit_->setMaxLength(6);
    colorEdit_->setAlignment(Qt::AlignCenter);
    colorEdit_->setPlaceholderText(tr("e.g. FF5C57"));
    auto* validator = new QRegularExpressionValidator(
        QRegularExpression(QStringLiteral("[0-9A-Fa-f]{0,6}")),
        colorEdit_);
    colorEdit_->setValidator(validator);
    connect(colorEdit_, &QLineEdit::textChanged, this, [this](const QString& text) {
        QString upper = text.toUpper();
        if (upper != text) {
            QSignalBlocker blocker(colorEdit_);
            colorEdit_->setText(upper);
        }
    });
    connect(colorEdit_, &QLineEdit::editingFinished, this, [this]() {
        const QString text = colorEdit_->text();
        if (text.size() != 6) {
            setColorEditText(currentColor_);
            return;
        }
        const QColor color = colorFromHex(text);
        if (!color.isValid()) {
            setColorEditText(currentColor_);
            return;
        }
        emitColor(color);
    });

    drawLayout->addWidget(colorEdit_);
    drawLayout->addStretch(1);

    pageMap_.insert(Mode::Pan, modeStack_->addWidget(navPage));
    pageMap_.insert(Mode::Draw, modeStack_->addWidget(drawPage));
    modeStack_->setCurrentIndex(pageMap_.value(Mode::Pan));

    layout->addWidget(modeStack_, 1);
    layout->addStretch(1);
}

void SlidePanel::setMode(Mode mode) {
    if (!modeButtons_) return;
    QAbstractButton* btn = modeButtons_->button(static_cast<int>(mode));
    if (!btn) return;

    QSignalBlocker blocker(modeButtons_);
    btn->setChecked(true);

    if (modeStack_) {
        const int idx = pageMap_.value(mode, pageMap_.value(Mode::Pan));
        modeStack_->setCurrentIndex(idx);
    }
}

void SlidePanel::setBrushWidth(double px) {
    if (!brushSpin_) return;
    QSignalBlocker blocker(brushSpin_);
    brushSpin_->setValue(px);
}

void SlidePanel::setBrushColor(const QColor& color) {
    if (!color.isValid()) return;
    if (color == currentColor_) return;
    currentColor_ = color;
    applyColorToButtons(color);
    setColorEditText(color);
}

void SlidePanel::handleModeButton(int id) {
    const auto mode = static_cast<Mode>(id);
    if (modeStack_) {
        const int idx = pageMap_.value(mode, pageMap_.value(Mode::Pan));
        modeStack_->setCurrentIndex(idx);
    }
    emit modeRequested(mode);
}

void SlidePanel::handleColorButton(int id) {
    if (!colorButtons_) return;
    QAbstractButton* btn = colorButtons_->button(id);
    if (!btn) return;
    const QString hex = btn->property("hex").toString();
    const QColor color = colorFromHex(hex);
    if (!color.isValid()) return;
    emitColor(color);
}

void SlidePanel::applyColorToButtons(const QColor& color) {
    if (!colorButtons_) return;
    const QString targetHex = hexFromColor(color);
    QSignalBlocker blocker(colorButtons_);
    bool matched = false;
    for (QAbstractButton* btn : colorButtons_->buttons()) {
        const QString hex = btn->property("hex").toString();
        const bool shouldCheck = hex.compare(targetHex, Qt::CaseInsensitive) == 0;
        btn->setChecked(shouldCheck);
        matched |= shouldCheck;
    }
    if (!matched) {
        for (QAbstractButton* btn : colorButtons_->buttons()) {
            btn->setChecked(false);
        }
    }
}

QColor SlidePanel::colorFromHex(const QString& hex) const {
    QString normalized = hex;
    normalized.remove(QLatin1Char('#'));
    if (normalized.size() != 6) return QColor();
    bool ok = false;
    const uint value = normalized.toUInt(&ok, 16);
    if (!ok) return QColor();
    return QColor(
        static_cast<int>((value >> 16) & 0xFF),
        static_cast<int>((value >> 8) & 0xFF),
        static_cast<int>(value & 0xFF)
    );
}

void SlidePanel::setColorEditText(const QColor& color) {
    if (!colorEdit_) return;
    const QString hex = hexFromColor(color);
    if (colorEdit_->text() == hex) return;
    QSignalBlocker blocker(colorEdit_);
    colorEdit_->setText(hex);
}

void SlidePanel::emitColor(const QColor& color) {
    if (!color.isValid()) return;
    if (color == currentColor_) {
        setColorEditText(color);
        applyColorToButtons(color);
        return;
    }
    currentColor_ = color;
    setColorEditText(color);
    applyColorToButtons(color);
    emit brushColorChanged(color);
}

} // namespace ui
