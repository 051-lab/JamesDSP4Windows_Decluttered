/**
 * JamesDSP for Windows - Custom Widgets
 * NoWheelSlider - Slider that ignores wheel/scroll events
 * EffectCard - Android-style collapsible effect container
 */

#ifndef WIDGETS_H
#define WIDGETS_H

#include <QSlider>
#include <QDialog>
#include <QWheelEvent>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QFrame>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QScrollArea>
#include <QVariantAnimation>
#include <QStyleOptionButton>

// ============================================================
// NoWheelSlider - Slider that ignores scroll wheel events
// ============================================================
class NoWheelSlider : public QSlider
{
public:
    explicit NoWheelSlider(Qt::Orientation orientation, QWidget* parent = nullptr)
        : QSlider(orientation, parent) { setFocusPolicy(Qt::StrongFocus); }
    explicit NoWheelSlider(QWidget* parent = nullptr)
        : QSlider(parent) { setFocusPolicy(Qt::StrongFocus); }
protected:
    void wheelEvent(QWheelEvent* event) override { event->ignore(); }
};

// ============================================================
// NoWheelComboBox - ComboBox that ignores scroll wheel events
// ============================================================
#include <QComboBox>
class NoWheelComboBox : public QComboBox
{
public:
    explicit NoWheelComboBox(QWidget* parent = nullptr) : QComboBox(parent) {
        setFocusPolicy(Qt::StrongFocus);
    }
protected:
    void wheelEvent(QWheelEvent* event) override {
        event->ignore(); 
    }
};

// ============================================================
// AnimatedSwitch - Material Design Toggle
// ============================================================
class AnimatedSwitch : public QCheckBox
{
    Q_OBJECT
    Q_PROPERTY(float offset READ offset WRITE setOffset)
public:
    explicit AnimatedSwitch(QWidget* parent = nullptr) : QCheckBox(parent), m_offset(0.0f) {
        setFixedSize(50, 30);
        setCursor(Qt::PointingHandCursor);
    }

    float offset() const { return m_offset; }
    void setOffset(float o) { m_offset = o; update(); }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        // Track
        QRect trackRect(0, 5, width(), 20);
        QColor trackColor = isChecked() ? QColor(142, 68, 173, 150) : QColor(60, 60, 60);
        p.setBrush(trackColor);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(trackRect, 10, 10);

        // Handle
        float handleSize = 26.0f;
        // If not animating (offset matches state roughly), snap to state for visual consistency on load
        // But better to trust offset if driven by animation. 
        // Force offset to match state if we never animated?
        // We will rely on showEvent to set initial state.
        
        float x = m_offset * (width() - handleSize);
        float y = (height() - handleSize) / 2.0f;
        
        QColor handleColor = isChecked() ? QColor(187, 134, 252) : QColor(180, 180, 180);
        if(!isEnabled()) handleColor = QColor(100, 100, 100);
        
        p.setBrush(handleColor);
        p.drawEllipse(QRectF(x, y, handleSize, handleSize));
    }
    
    void showEvent(QShowEvent* e) override {
        QCheckBox::showEvent(e);
        m_offset = isChecked() ? 1.0f : 0.0f;
    }

    void checkStateSet() override {
        QCheckBox::checkStateSet();
        // Animate
        QPropertyAnimation* anim = new QPropertyAnimation(this, "offset", this);
        anim->setDuration(200);
        anim->setStartValue(m_offset);
        anim->setEndValue(isChecked() ? 1.0f : 0.0f);
        anim->setEasingCurve(QEasingCurve::OutQuad);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
    
    bool hitButton(const QPoint &pos) const override {
        return rect().contains(pos);
    }

private:
    float m_offset;
};


// ============================================================
// EffectCard - Android-style collapsible card
// ============================================================
class EffectCard : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(int contentHeight READ contentHeight WRITE setContentHeight)
public:
    AnimatedSwitch* enableSwitch;
    QWidget* contentArea;
    QWidget* header; // Exposed for style updates
    QVBoxLayout* contentLayout;
    QLabel* titleLabel;

    int contentHeight() const { return contentArea->maximumHeight(); }
    void setContentHeight(int h) { contentArea->setMaximumHeight(h); }

    explicit EffectCard(const QString& title, const QIcon& icon = QIcon(), QWidget* parent = nullptr)
        : QFrame(parent)
    {
        // Card Style - Rounded corners, dark background
        setStyleSheet(R"(
            EffectCard {
                background-color: #121212;
                border: none;
                border-radius: 12px;
                margin-top: 4px;
                margin-bottom: 4px;
            }
        )");

        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(0,0,0,0);
        mainLayout->setSpacing(0);

        // --- Header ---
        header = new QWidget(this);
        header->setObjectName("header");
        // Default Inactive Style
        header->setStyleSheet("background-color: #1e1e1e; border-radius: 12px;");
        
        QHBoxLayout* headerLayout = new QHBoxLayout(header);
        headerLayout->setContentsMargins(16, 12, 16, 12);
        
        // Icon
        if (!icon.isNull()) {
            QLabel* iconLabel = new QLabel(this);
            iconLabel->setPixmap(icon.pixmap(24, 24));
            headerLayout->addWidget(iconLabel);
            headerLayout->addSpacing(10);
        }

        // Title
        titleLabel = new QLabel(title, this);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 500; color: #aaa;"); // Inactive text color
        headerLayout->addWidget(titleLabel);
        
        headerLayout->addStretch();

        // Switch
        enableSwitch = new AnimatedSwitch(this);
        headerLayout->addWidget(enableSwitch);

        mainLayout->addWidget(header);

        // --- Content ---
        contentArea = new QWidget(this);
        contentLayout = new QVBoxLayout(contentArea);
        contentLayout->setContentsMargins(16, 16, 16, 16); 
        contentLayout->setSpacing(16);
        
        // Add specific Android-style slider styling for children of this card
        contentArea->setStyleSheet(R"(
            QSlider::groove:horizontal { border: none; height: 4px; background: #333; border-radius: 2px; }
            QSlider::sub-page:horizontal { background: #bb86fc; border-radius: 2px; }
            QSlider::handle:horizontal { 
                background: #bb86fc; 
                width: 18px; 
                height: 18px; 
                margin: -7px 0; 
                border-radius: 9px; 
                border: 2px solid #1a1a1a; /* visual separation */
            }
            QSlider::handle:horizontal:hover { transform: scale(1.1); }
            QLabel { color: #ddd; }
        )");

        mainLayout->addWidget(contentArea);
        
        // Connect switch to Expand/Collapse and Style
        QObject::connect(enableSwitch, &QCheckBox::toggled, [this](bool checked){
            updateState(checked);
        });
        
        // Initial state
        contentArea->setVisible(false);
        contentArea->setMaximumHeight(0);
        updateState(false);
    }

    void updateState(bool checked) {
        if (checked) {
            // Active Style
            header->setStyleSheet(R"(
                background-color: #37234a; 
                border-top-left-radius: 12px; 
                border-top-right-radius: 12px;
                border-bottom-left-radius: 0px;
                border-bottom-right-radius: 0px;
            )");
            titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #ffffff;");
            
            // Expand Animation
            contentArea->setVisible(true);
            // Measure target height
            int targetHeight = contentArea->sizeHint().height();
            // If sizeHint is tight, add some buffer or ensure widgets are sizeable
            if (targetHeight < 50) targetHeight = 300; // Fallback? No, better relying on layout
            
            // Re-measure properly
            // Force layout to calculate size
            contentArea->setMaximumHeight(16777215); // Unbound
            contentArea->adjustSize();
            targetHeight = contentArea->sizeHint().height();
            
            QPropertyAnimation* anim = new QPropertyAnimation(this, "contentHeight");
            anim->setDuration(300);
            anim->setStartValue(0);
            anim->setEndValue(targetHeight);
            anim->setEasingCurve(QEasingCurve::OutQuad);
            anim->start(QAbstractAnimation::DeleteWhenStopped);
            
            // Ensure end state fixes height to unbound after anim? 
            // Actually better to keep it unbound, but QPropertyAnimation works on specific property.
            // Let's use QVariantAnimation lambda if we want robust layout handling
            
             connect(anim, &QPropertyAnimation::finished, [this](){
                 contentArea->setMaximumHeight(16777215); // Unlock height
             });

        } else {
            // Inactive Style
            header->setStyleSheet("background-color: #1e1e1e; border-radius: 12px;");
            titleLabel->setStyleSheet("font-size: 16px; font-weight: 500; color: #aaa;");
            
            // Collapse Animation
            int currentH = contentArea->height();
            QPropertyAnimation* anim = new QPropertyAnimation(this, "contentHeight");
            anim->setDuration(300);
            anim->setStartValue(currentH);
            anim->setEndValue(0);
            anim->setEasingCurve(QEasingCurve::OutQuad);
            anim->start(QAbstractAnimation::DeleteWhenStopped);
            
            connect(anim, &QPropertyAnimation::finished, [this](){
                contentArea->setVisible(false);
            });
        }
    }

    void addWidget(QWidget* w) {
        contentLayout->addWidget(w);
    }
    
    void addLayout(QLayout* l) {
        contentLayout->addLayout(l);
    }
};

#endif // WIDGETS_H
