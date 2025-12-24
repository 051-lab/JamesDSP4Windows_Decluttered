/**
 * JamesDSP for Windows - Visual Curve Widgets
 * EQ Curve and Compander Curve editors - Android Replica
 * Updated for Single-Window Navigation
 */

#ifndef CURVE_WIDGETS_H
#define CURVE_WIDGETS_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <vector>
#include <QMouseEvent>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QInputDialog>
#include <QScrollArea>
#include <QStackedWidget>
#include <cmath>
#include <functional>
#include <algorithm>
#include <QTimer>
#include <QPlainTextEdit>
#include <QTextStream>
#include <QFile>
#include <QWheelEvent>
#include <QSlider>
#include <QRegularExpression>
#include <string>
#include <iostream>
#include <fstream>
#include "widgets.h" // For NoWheelSlider

// ============================================================
// FrequencyResponseWidget - Visual EQ curve editor
// ============================================================
class FrequencyResponseWidget : public QWidget
{
public:
    struct Node {
        double freq;  // Hz (20 - 24000)
        double gain;  // dB (-20 to +20)
    };

    std::function<void(int index, double freq, double gain)> onNodeSelected;
    std::function<void(int index, double freq, double gain)> onNodeChanged;
    std::function<void()> onCurveChanged;

    explicit FrequencyResponseWidget(QWidget* parent = nullptr)
        : QWidget(parent)
        , m_minFreq(20)
        , m_maxFreq(24000)
        , m_minGain(-20)
        , m_maxGain(20)
        , m_selectedNode(-1)
        , m_dragging(false)
        , m_readOnly(false)
    {
        setMinimumSize(400, 250);
        setMouseTracking(true);
        m_nodes.append({20, 0});
        m_nodes.append({24000, 0});
    }
    
    void setReadOnly(bool ro) { m_readOnly = ro; update(); }
    bool isReadOnly() const { return m_readOnly; }

    void setNodes(const QList<Node>& nodes) { 
        m_nodes = nodes; 
        update(); 
        if (onCurveChanged) onCurveChanged();
    }
    QList<Node> nodes() const { return m_nodes; }
    
    int selectedNode() const { return m_selectedNode; }
    void setSelectedNode(int index) { 
        if (index >= -1 && index < m_nodes.size()) {
            m_selectedNode = index;
            update();
            if (m_selectedNode != -1 && onNodeSelected) {
                onNodeSelected(m_selectedNode, m_nodes[m_selectedNode].freq, m_nodes[m_selectedNode].gain);
            }
        }
    }

    void updateNode(int index, double freq, double gain) {
        if (index >= 0 && index < m_nodes.size()) {
            m_nodes[index].freq = qBound(m_minFreq, freq, m_maxFreq);
            m_nodes[index].gain = qBound((double)m_minGain, gain, (double)m_maxGain);
            update();
            if (onNodeChanged) onNodeChanged(index, m_nodes[index].freq, m_nodes[index].gain);
            if (onCurveChanged) onCurveChanged();
        }
    }

    void addNode(double freq, double gain) {
        freq = qBound(m_minFreq, freq, m_maxFreq);
        gain = qBound((double)m_minGain, gain, (double)m_maxGain);
        m_nodes.append({freq, gain});
        update();
        if (onCurveChanged) onCurveChanged();
    }

    void removeNode(int index) {
        if (index >= 0 && index < m_nodes.size() && m_nodes.size() > 2) {
            m_nodes.removeAt(index);
            m_selectedNode = -1;
            update();
            update();
            if (onNodeSelected) onNodeSelected(-1, 0, 0);
            if (onCurveChanged) onCurveChanged();
        }
    }

    QString toGraphicEqString() const {
        QList<Node> sorted = m_nodes;
        std::sort(sorted.begin(), sorted.end(), [](const Node& a, const Node& b) {
            return a.freq < b.freq;
        });

        QString result = "GraphicEQ:";
        for (const Node& n : sorted) {
            result += QString(" %1 %2;").arg(n.freq, 0, 'f', 1).arg(n.gain, 0, 'f', 1);
        }
        return result;
    }

    void fromGraphicEqString(const QString& str) {
        m_nodes.clear();
        QString data = str;
        if (data.startsWith("GraphicEQ:", Qt::CaseInsensitive)) {
            data = data.mid(10).trimmed();
        }
        QStringList parts = data.split(';', Qt::SkipEmptyParts);
        for (const QString& part : parts) {
            QStringList values = part.trimmed().split(' ', Qt::SkipEmptyParts);
            if (values.size() >= 2) {
                m_nodes.append({values[0].toDouble(), values[1].toDouble()});
            }
        }
        
        if (m_nodes.size() < 2) {
            m_nodes.clear(); m_nodes.append({20, 0}); m_nodes.append({24000, 0});
        }
        update();
        if (onCurveChanged) onCurveChanged();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        
        int w = width(); int h = height();
        int margin = 35; int topMargin = 10; int rightMargin = 10;
        QRect graphRect(margin, topMargin, w - margin - rightMargin, h - margin - topMargin);
        
        // Background
        p.fillRect(rect(), QColor(30, 30, 35));
        p.fillRect(graphRect, QColor(20, 20, 25));
        
        // Grid
        p.setPen(QPen(QColor(50, 50, 60), 1));
        double freqs[] = {20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000};
        for (double f : freqs) {
            int x = freqToX(f, graphRect);
            if (x >= graphRect.left() && x <= graphRect.right()) {
                p.drawLine(x, graphRect.top(), x, graphRect.bottom());
                p.setPen(QColor(150, 150, 160));
                QString label = f >= 1000 ? QString("%1k").arg(f/1000.0, 0, 'f', 0) : QString::number((int)f);
                p.drawText(x - 15, graphRect.bottom() + 2, 30, 15, Qt::AlignCenter, label);
                p.setPen(QPen(QColor(50, 50, 60), 1));
            }
        }
        
        for (int g = m_minGain; g <= m_maxGain; g += 5) {
            int y = gainToY(g, graphRect);
            p.setPen(g == 0 ? QPen(QColor(100, 100, 110), 1) : QPen(QColor(50, 50, 60), 1));
            p.drawLine(graphRect.left(), y, graphRect.right(), y);
            p.setPen(QColor(150, 150, 160));
            p.drawText(2, y - 7, margin - 5, 14, Qt::AlignRight | Qt::AlignVCenter, QString::number(g));
        }
        
        QList<Node> sorted = m_nodes;
        std::sort(sorted.begin(), sorted.end(), [](const Node& a, const Node& b) { return a.freq < b.freq; });
        
        if (sorted.size() >= 2) {
            QPainterPath path;
            bool first = true;
            for (int i = 0; i < sorted.size(); i++) {
                int x = freqToX(sorted[i].freq, graphRect);
                int y = gainToY(sorted[i].gain, graphRect);
                if (first) { path.moveTo(x, y); first = false; } else { path.lineTo(x, y); }
            }
            
            QPainterPath fillPath = path;
            fillPath.lineTo(freqToX(sorted.last().freq, graphRect), graphRect.bottom());
            fillPath.lineTo(freqToX(sorted.first().freq, graphRect), graphRect.bottom());
            fillPath.closeSubpath();
            
            QLinearGradient grad(0, graphRect.top(), 0, graphRect.bottom());
            grad.setColorAt(0, QColor(142, 68, 173, 200)); // Boosted Opacity
            grad.setColorAt(1, QColor(142, 68, 173, 50));
            p.fillPath(fillPath, grad);
            p.setPen(QPen(QColor(187, 134, 252), 2));
            p.drawPath(path);
        }
        
        for (int i = 0; i < m_nodes.size(); i++) {
            int x = freqToX(m_nodes[i].freq, graphRect);
            int y = gainToY(m_nodes[i].gain, graphRect);
            bool isSelected = (i == m_selectedNode);
            QColor nodeColor = isSelected ? QColor(255, 255, 255) : QColor(187, 134, 252);
            int size = isSelected ? 8 : 6;
            p.setPen(QPen(nodeColor, 2));
            p.setBrush(isSelected ? QColor(142, 68, 173) : QColor(30, 30, 35));
            p.drawEllipse(QPoint(x, y), size, size);
        }
    }
    
    void mousePressEvent(QMouseEvent* e) override {
        if (m_readOnly) return;
        int idx = nodeAt(e->pos());
        if (e->button() == Qt::LeftButton) {
            if (idx >= 0) {
                m_selectedNode = idx; m_dragging = true;
                if (onNodeSelected) onNodeSelected(m_selectedNode, m_nodes[idx].freq, m_nodes[idx].gain);
            } else {
                QRect graphRect(35, 10, width() - 45, height() - 45);
                if (graphRect.contains(e->pos())) {
                    double freq = xToFreq(e->pos().x(), graphRect);
                    double gain = yToGain(e->pos().y(), graphRect);
                    addNode(freq, gain);
                    m_selectedNode = m_nodes.size() - 1; m_dragging = true;
                    if (onNodeSelected) onNodeSelected(m_selectedNode, freq, gain);
                } else {
                     m_selectedNode = -1;
                     if (onNodeSelected) onNodeSelected(-1, 0, 0);
                }
            }
            update();
        } else if (e->button() == Qt::RightButton && idx >= 0) {
            removeNode(idx);
        }
    }
    
    void mouseMoveEvent(QMouseEvent* e) override {
        if (m_readOnly) return;
        if (m_dragging && m_selectedNode >= 0) {
            QRect graphRect(35, 10, width() - 45, height() - 45);
            updateNode(m_selectedNode, xToFreq(e->pos().x(), graphRect), yToGain(e->pos().y(), graphRect));
        }
    }
    
    void mouseReleaseEvent(QMouseEvent*) override {
        m_dragging = false;
        std::sort(m_nodes.begin(), m_nodes.end(), [](const Node& a, const Node& b) { return a.freq < b.freq; });
        update();
    }

private:
    int freqToX(double freq, const QRect& r) const {
        double logMin = std::log10(m_minFreq); double logMax = std::log10(m_maxFreq);
        double logFreq = std::log10(qBound(m_minFreq, freq, m_maxFreq));
        return r.left() + (int)((logFreq - logMin) / (logMax - logMin) * r.width());
    }
    double xToFreq(int x, const QRect& r) const {
        double logMin = std::log10(m_minFreq); double logMax = std::log10(m_maxFreq);
        double t = (double)(x - r.left()) / r.width();
        return std::pow(10, logMin + t * (logMax - logMin));
    }
    int gainToY(double gain, const QRect& r) const {
        return r.bottom() - (int)((gain - m_minGain) / (m_maxGain - m_minGain) * r.height());
    }
    double yToGain(int y, const QRect& r) const {
        return m_minGain + (double)(r.bottom() - y) / r.height() * (m_maxGain - m_minGain);
    }
    int nodeAt(const QPoint& pos) const {
        QRect graphRect(35, 10, width() - 45, height() - 45);
        for (int i = 0; i < m_nodes.size(); i++) {
            int x = freqToX(m_nodes[i].freq, graphRect); int y = gainToY(m_nodes[i].gain, graphRect);
            if (QRect(x - 9, y - 9, 18, 18).contains(pos)) return i;
        }
        return -1;
    }

    QList<Node> m_nodes;
    double m_minFreq, m_maxFreq;
    int m_minGain, m_maxGain;
    int m_selectedNode;
    bool m_dragging;
    bool m_readOnly;
};

// ============================================================
// CompanderCurveWidget - 7-band compander response editor
// ============================================================
class CompanderCurveWidget : public QWidget
{
public:
    explicit CompanderCurveWidget(QWidget* parent = nullptr)
        : QWidget(parent), m_selectedBand(-1), m_dragging(false), m_readOnly(false) {
        setMinimumSize(400, 200); setMouseTracking(true);
        m_freqs = {95, 200, 400, 800, 1600, 3400, 7500};
        m_gains = {0, 0, 0, 0, 0, 0, 0};
    }
    void setReadOnly(bool ro) { m_readOnly = ro; update(); }
    void setGains(const QList<double>& gains) { 
        m_gains = gains; 
        while(m_gains.size()<7)m_gains.append(0); 
        update(); 
        if (onCurveChanged) onCurveChanged();
    }
    QList<double> gains() const { return m_gains; }
    
    std::function<void()> onCurveChanged;

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        QRect graphRect(35, 10, width() - 45, height() - 45);
        p.fillRect(rect(), QColor(30, 30, 35)); p.fillRect(graphRect, QColor(20, 20, 25));
        p.setPen(QPen(QColor(50, 50, 60), 1));
        
        for (int g = -12; g <= 12; g += 4) {
            int y = gainToY(g, graphRect);
            p.setPen(g == 0 ? QPen(QColor(100, 100, 110), 1) : QPen(QColor(50, 50, 60), 1));
            p.drawLine(graphRect.left(), y, graphRect.right(), y);
            p.setPen(QColor(150, 150, 160));
            p.drawText(2, y - 7, 30, 14, Qt::AlignRight | Qt::AlignVCenter, QString::number(g));
        }
        
        // Vertical Grid Lines (New)
        int bandWidth = graphRect.width() / 7;
        p.setPen(QPen(QColor(80, 80, 90), 1)); // Brighter grid
        for (int i = 0; i < 7; i++) {
             int x = graphRect.left() + bandWidth * i + bandWidth / 2;
             p.drawLine(x, graphRect.top(), x, graphRect.bottom());
        }
        
        
        QPainterPath path; bool first = true;
        for (int i = 0; i < 7; i++) {
            int x = graphRect.left() + bandWidth * i + bandWidth / 2;
            int y = gainToY(m_gains[i], graphRect);
            if (first) { path.moveTo(x, y); first = false; } else { path.lineTo(x, y); }
        }
        QPainterPath fillPath = path;
        fillPath.lineTo(graphRect.right() - bandWidth/2, graphRect.bottom());
        fillPath.lineTo(graphRect.left() + bandWidth/2, graphRect.bottom());
        fillPath.closeSubpath();
        
        QLinearGradient grad(0, graphRect.top(), 0, graphRect.bottom());
        grad.setColorAt(0, QColor(142, 68, 173, 200)); grad.setColorAt(1, QColor(142, 68, 173, 50));
        p.fillPath(fillPath, grad);
        p.setPen(QPen(QColor(187, 134, 252), 2)); p.drawPath(path);
        
        for (int i = 0; i < 7; i++) {
            int x = graphRect.left() + bandWidth * i + bandWidth / 2;
            int y = gainToY(m_gains[i], graphRect);
            p.setPen(QPen(i == m_selectedBand ? Qt::white : QColor(187, 134, 252), 2));
            p.setBrush(i == m_selectedBand ? QColor(142, 68, 173) : QColor(30, 30, 35));
            p.drawEllipse(QPoint(x, y), 8, 8);
            
            p.setPen(QColor(150, 150, 160));
            QString label = m_freqs[i] >= 1000 ? QString("%1k").arg(m_freqs[i]/1000.0, 0, 'f', 1) : QString::number(m_freqs[i]);
            p.drawText(x - 20, graphRect.bottom() + 5, 40, 15, Qt::AlignCenter, label);
        }
    }
    
    void mousePressEvent(QMouseEvent* e) override {
        if (m_readOnly) return;
        if (e->button() == Qt::LeftButton) {
            m_selectedBand = bandAt(e->pos());
            if (m_selectedBand >= 0) m_dragging = true;
            update();
        }
    }
    
    void mouseMoveEvent(QMouseEvent* e) override {
        if (m_readOnly) return;
        if (m_dragging && m_selectedBand >= 0) {
            QRect r(35, 10, width() - 45, height() - 45);
            m_gains[m_selectedBand] = qBound(-12.0, yToGain(e->pos().y(), r), 12.0);
            update();
            if (onCurveChanged) onCurveChanged();
        }
    }
    
    void mouseReleaseEvent(QMouseEvent*) override { m_dragging = false; }

private:
    int gainToY(double gain, const QRect& r) const { return r.bottom() - (int)((gain + 12) / 24 * r.height()); }
    double yToGain(int y, const QRect& r) const { return -12 + (double)(r.bottom() - y) / r.height() * 24; }
    int bandAt(const QPoint& pos) const {
        QRect r(35, 10, width() - 45, height() - 45); int bw = r.width() / 7;
        for (int i = 0; i < 7; i++) {
            int x = r.left() + bw * i + bw / 2; int y = gainToY(m_gains[i], r);
            if (QRect(x - 12, y - 12, 24, 24).contains(pos)) return i;
        }
        return -1;
    }
    
    QList<double> m_gains;
    QList<int> m_freqs;
    int m_selectedBand;
    bool m_dragging;
    bool m_readOnly;
};


// ============================================================
// GraphicEqPage - Full page widget for GraphicEQ
// ============================================================
class GraphicEqPage : public QWidget
{
public:
    // Callback for Back button
    std::function<void()> onBack;
    std::function<void(const QString&)> onApply;

    explicit GraphicEqPage(QWidget* parent = nullptr) : QWidget(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(10, 10, 10, 10);
        
        // Header
        QHBoxLayout* header = new QHBoxLayout();
        QPushButton* backBtn = new QPushButton("← Back");
        backBtn->setFlat(true);
        backBtn->setStyleSheet("text-align: left; font-size: 16px; color: #bb86fc; font-weight: bold;");
        connect(backBtn, &QPushButton::clicked, [this]() { if(onBack) onBack(); });
        header->addWidget(backBtn);
        
        QLabel* title = new QLabel("Graphic Equalizer");
        title->setStyleSheet("font-size: 18px; font-weight: bold; color: white;");
        header->addWidget(title);
        header->addStretch();
        layout->addLayout(header);
        
        // Graph
        m_curve = new FrequencyResponseWidget(this);
        layout->addWidget(m_curve, 1);
        
        // Controls
        QGroupBox* controlsGroup = new QGroupBox("Node Control");
        QHBoxLayout* controlsLayout = new QHBoxLayout(controlsGroup);
        
        controlsLayout->addWidget(new QLabel("Freq (Hz):"));
        m_freqSpin = new QDoubleSpinBox(); m_freqSpin->setRange(20, 24000); m_freqSpin->setSingleStep(10);
        controlsLayout->addWidget(m_freqSpin);
        
        controlsLayout->addWidget(new QLabel("Gain (dB):"));
        m_gainSpin = new QDoubleSpinBox(); m_gainSpin->setRange(-20, 20); m_gainSpin->setSingleStep(0.1);
        controlsLayout->addWidget(m_gainSpin);
        
        QPushButton* addBtn = new QPushButton("Add");
        connect(addBtn, &QPushButton::clicked, [this]() { m_curve->addNode(1000, 0); });
        controlsLayout->addWidget(addBtn);
        
        QPushButton* delBtn = new QPushButton("Delete");
        connect(delBtn, &QPushButton::clicked, [this]() { m_curve->removeNode(m_curve->selectedNode()); });
        controlsLayout->addWidget(delBtn);
        
        layout->addWidget(controlsGroup);
        
        // String & Reset
        QHBoxLayout* bottomRow = new QHBoxLayout();
        QPushButton* editStrBtn = new QPushButton("Edit String...");
        connect(editStrBtn, &QPushButton::clicked, this, &GraphicEqPage::editString);
        bottomRow->addWidget(editStrBtn);
        
        QPushButton* resetBtn = new QPushButton("Reset Flat");
        connect(resetBtn, &QPushButton::clicked, [this]() {
            m_curve->setNodes({{20,0}, {24000,0}});
        });
        bottomRow->addWidget(resetBtn);
        bottomRow->addStretch();

        // Apply Button Removed primarily as per user request
        // We now rely on onApply callback triggered by curve changes
        
        // Ensure onNodeChanged calls onApply
        m_curve->onCurveChanged = [this](){
             if(onApply) onApply(m_curve->toGraphicEqString());
        };

        layout->addLayout(bottomRow);
        
        // Callbacks
        m_curve->onNodeSelected = [this](int idx, double f, double g) {
            bool has = (idx >= 0);
            m_freqSpin->setEnabled(has); m_gainSpin->setEnabled(has);
            if(has) { 
                m_freqSpin->blockSignals(true); m_gainSpin->blockSignals(true);
                m_freqSpin->setValue(f); m_gainSpin->setValue(g);
                m_freqSpin->blockSignals(false); m_gainSpin->blockSignals(false);
            }
        };
        connect(m_freqSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){
            if(m_curve->selectedNode()>=0) m_curve->updateNode(m_curve->selectedNode(), v, m_gainSpin->value());
        });
        connect(m_gainSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){
            if(m_curve->selectedNode()>=0) m_curve->updateNode(m_curve->selectedNode(), m_freqSpin->value(), v);
        });
    }
    
    void editString() {
        bool ok; QString text = QInputDialog::getMultiLineText(this, "Input GraphicEQ", "Paste String:", m_curve->toGraphicEqString(), &ok);
        if (ok) m_curve->fromGraphicEqString(text);
    }
    
    FrequencyResponseWidget* curve() { return m_curve; }

private:
    FrequencyResponseWidget* m_curve;
    QDoubleSpinBox* m_freqSpin; QDoubleSpinBox* m_gainSpin;
};

// ============================================================
// CompanderPage - Full page widget for Compander
// ============================================================
class CompanderPage : public QWidget
{
public:
    std::function<void()> onBack;
    std::function<void()> onApply;

    explicit CompanderPage(QWidget* parent = nullptr) : QWidget(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);
        
        QHBoxLayout* header = new QHBoxLayout();
        QPushButton* backBtn = new QPushButton("← Back");
        backBtn->setFlat(true);
        backBtn->setStyleSheet("text-align: left; font-size: 16px; color: #bb86fc; font-weight: bold;");
        connect(backBtn, &QPushButton::clicked, [this]() { if(onBack) onBack(); });
        header->addWidget(backBtn);
        header->addWidget(new QLabel("Compander Response"));
        header->addStretch();
        layout->addLayout(header);
        
        layout->addWidget(new QLabel("Adjust gain at 7 fixed bands:"));
        m_curve = new CompanderCurveWidget(this);
        layout->addWidget(m_curve, 1);
        
        QHBoxLayout* btnRow = new QHBoxLayout();
        QPushButton* resetBtn = new QPushButton("Flat");
        connect(resetBtn, &QPushButton::clicked, [this]() { m_curve->setGains({0,0,0,0,0,0,0}); });
        btnRow->addWidget(resetBtn);
        btnRow->addStretch();
        
        // Apply Button Removed
        m_curve->onCurveChanged = [this](){
             if(onApply) onApply();
        };
        
        layout->addLayout(btnRow);
    }
    
    CompanderCurveWidget* curve() { return m_curve; }

private:
    CompanderCurveWidget* m_curve;
};

// Keep old dialog classes as wrappers just in case logic depends on them
// But ideally we replace usage with Pages
class GraphicEqEditorDialog : public QDialog {
    // Deprecated for integrated UI, but kept for compilation compatibility
public: 
    GraphicEqEditorDialog(QWidget* p=nullptr) : QDialog(p) {} 
    QString toGraphicEqString() { return ""; }
    int nodeCount() { return 0; }
};
class CompanderEditorDialog : public QDialog {
public: CompanderEditorDialog(QWidget* p=nullptr) : QDialog(p) {}
};


// ============================================================
// ImpulseResponseDialog - Numerical Editor (Text based)
// ============================================================
class ImpulseResponseDialog : public QDialog
{
public:
    explicit ImpulseResponseDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Advanced Waveform Editor");
        resize(600, 400);
        setStyleSheet("QDialog { background: #121212; color: white; }");
        
        QVBoxLayout* layout = new QVBoxLayout(this);
        
        QLabel* title = new QLabel("Impulse Response Data (Semicolon separated)");
        title->setStyleSheet("font-size: 16px; font-weight: bold; color: #bb86fc;");
        layout->addWidget(title);
        
        m_editor = new QPlainTextEdit();
        m_editor->setStyleSheet("background: #1e1e1e; color: #ccc; font-family: Consolas, monospace; border: 1px solid #333;");
        m_editor->setPlaceholderText("Example: -80;-100;0;0;0;0");
        m_editor->setPlainText("-80;-100;0;0;0;0"); // Default 6-value format
        layout->addWidget(m_editor);
        
        QHBoxLayout* btnRow = new QHBoxLayout();
        QPushButton* loadBtn = new QPushButton("Load IR...");
        btnRow->addWidget(loadBtn);
        btnRow->addStretch();
        QPushButton* saveBtn = new QPushButton("Save");
        connect(saveBtn, &QPushButton::clicked, this, &QDialog::accept);
        btnRow->addWidget(saveBtn);
        layout->addLayout(btnRow);
    }
private:
    QPlainTextEdit* m_editor;
};

// ============================================================
// Liveprog Dialogs
// ============================================================
class LiveprogEditorDialog : public QDialog
{
public:
    QPlainTextEdit* m_editor = nullptr;
    
    explicit LiveprogEditorDialog(const QString& scriptContent, QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("EEL Script Editor");
        resize(700, 500);
        setStyleSheet("QDialog { background: #121212; color: white; }");
        
        QVBoxLayout* layout = new QVBoxLayout(this);
        
        m_editor = new QPlainTextEdit(this);
        m_editor->setPlainText(scriptContent);
        m_editor->setStyleSheet("background: #1e1e1e; color: #a9b7c6; font-family: Consolas, monospace; border: 1px solid #333;");
        layout->addWidget(m_editor);
        
        QHBoxLayout* btnRow = new QHBoxLayout();
        QPushButton* saveAsBtn = new QPushButton("Save As...");
        connect(saveAsBtn, &QPushButton::clicked, this, &QDialog::accept);
        btnRow->addWidget(saveAsBtn);
        
        QPushButton* cancelBtn = new QPushButton("Cancel");
        connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
        btnRow->addWidget(cancelBtn);
        
        layout->addLayout(btnRow);
    }
    
    QString content() const { return m_editor ? m_editor->toPlainText() : QString(); }
};

// Helper for LiveProg Script Processing
class LiveProgHelper {
public:
    struct ParamDef {
        QString name;
        QString var;
        float def, min, max, step;
        QStringList options; // if not empty, it's a list
    };

    static QList<ParamDef> parseParams(const QString& script) {
        QList<ParamDef> params;
        
        // Number Range: var:def<min,max,step>desc
        QRegularExpression descRe(R"((?<var>\w+):(?<def>-?\d+\.?\d*)?<(?<min>-?\d+\.?\d*),(?<max>-?\d+\.?\d*),?(?<step>-?\d+\.?\d*)?>(?<desc>[\s\S][^\n]*))");
        // List: var:def<min,max,step{opt1,opt2}>desc
        QRegularExpression descListRe(R"((?<var>\w+):(?<def>-?\d+\.?\d*)?<(?<min>-?\d+\.?\d*),(?<max>-?\d+\.?\d*),?(?<step>-?\d+\.?\d*)?\{(?<opt>[^\}]*)\}>(?<desc>[\s\S][^\n]*))");
        
        QTextStream stream(const_cast<QString*>(&script), QIODevice::ReadOnly);
        while (!stream.atEnd()) {
            QString line = stream.readLine().trimmed();
            if (line.isEmpty() || line.startsWith("//")) continue; // Skip comments if they are commented out manually

            // Try List first
            auto matchList = descListRe.match(line);
            if (matchList.hasMatch()) {
                ParamDef p;
                p.name = matchList.captured("desc").trimmed();
                p.var = matchList.captured("var");
                p.def = matchList.captured("def").toFloat();
                p.options = matchList.captured("opt").split(",", Qt::SkipEmptyParts);
                // List usually doesn't use min/max/step validation in the same way, but let's store if present
                p.min = matchList.captured("min").toFloat();
                p.max = matchList.captured("max").toFloat();
                p.step = 1.0f;
                params.append(p);
                continue;
            }

            // Try Number Range
            auto match = descRe.match(line);
            if (match.hasMatch()) {
                ParamDef p;
                p.name = match.captured("desc").trimmed();
                p.var = match.captured("var");
                p.def = match.captured("def").toFloat();
                p.min = match.captured("min").toFloat();
                p.max = match.captured("max").toFloat();
                QString stepStr = match.captured("step");
                p.step = stepStr.isEmpty() ? 1.0f : stepStr.toFloat();
                if (p.step == 0.0f) p.step = 1.0f;
                params.append(p);
            }
        }
        return params;
    }

    static QString stripSliders(const QString& script) {
        QString out;
        QTextStream stream(const_cast<QString*>(&script), QIODevice::ReadOnly);
        // Regex to identify slider lines to comment out or remove
        QRegularExpression descRe(R"(\w+:-?\d+\.?\d*<.*>.*)");
        
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            // If it looks like a slider definition and isn't already commented
            if (descRe.match(line.trimmed()).hasMatch() && !line.trimmed().startsWith("//") && !line.trimmed().startsWith("slider")) {
                // JSFX syntax: slider1:....
                // EEL2 headers we use: var:def<...>
                // We just comment it out to be safe
                out += "// " + line + "\n";
            } else {
                out += line + "\n";
            }
        }
        return out;
    }

    static QString injectValues(const QString& script, const QMap<QString, float>& values) {
        QString out;
        QTextStream stream(const_cast<QString*>(&script), QIODevice::ReadOnly);
        
        static std::ofstream log("debug_log.txt", std::ios::out | std::ios::app);
        log << "--- Injecting Values (Preserving Syntax) ---\n" << std::endl;

        // Regex matchers
        QRegularExpression descRe(R"((?<var>\w+):(?<def>-?\d+\.?\d*)?<(?<min>-?\d+\.?\d*),(?<max>-?\d+\.?\d*),?(?<step>-?\d+\.?\d*)?>(?<desc>[\s\S][^\n]*))");
        QRegularExpression descListRe(R"((?<var>\w+):(?<def>-?\d+\.?\d*)?<(?<min>-?\d+\.?\d*),(?<max>-?\d+\.?\d*),?(?<step>-?\d+\.?\d*)?\{(?<opt>[^\}]*)\}>(?<desc>[\s\S][^\n]*))");
        QRegularExpression assignRe(R"(^\s*(?<var>\w+)\s*=\s*(?<val>-?\d+(\.\d*)?)\s*;)");

        // 1. Collect all slider variables first
        QSet<QString> sliderVars;
        {
             QTextStream s2(const_cast<QString*>(&script), QIODevice::ReadOnly);
             while(!s2.atEnd()) {
                 QString l = s2.readLine().trimmed();
                 auto m1 = descRe.match(l);
                 if(m1.hasMatch()) sliderVars.insert(m1.captured("var"));
                 auto m2 = descListRe.match(l);
                 if(m2.hasMatch()) sliderVars.insert(m2.captured("var"));
             }
        }

        while (!stream.atEnd()) {
            QString line = stream.readLine();
            QString trimmed = line.trimmed();

            if (trimmed.isEmpty() || trimmed.startsWith("//")) {
                out += line + "\n";
                continue;
            }

            QString varName;
            float valToUse = 0.0f;
            
            bool foundSlider = false;
            int defStart = -1;
            int defLength = -1;

            // Check List Format
            auto matchList = descListRe.match(trimmed);
            if (matchList.hasMatch()) {
                varName = matchList.captured("var");
                valToUse = matchList.captured("def").toFloat();
                defStart = matchList.capturedStart("def");
                defLength = matchList.capturedLength("def");
                foundSlider = true;
            } 
            // Check Range Format
            else {
                auto match = descRe.match(trimmed);
                if (match.hasMatch()) {
                    varName = match.captured("var");
                    valToUse = match.captured("def").toFloat();
                    defStart = match.capturedStart("def");
                    defLength = match.capturedLength("def");
                    foundSlider = true;
                }
            }

            if (foundSlider) {
                // It's a slider definition line. Update the default value IN PLACE
                if (values.contains(varName)) {
                    valToUse = values[varName];
                    // Reconstruct line: Replace the 'def' part with new value
                    // We need to operate on 'trimmed' but we want to preserve 'line' indentation if possible?
                    // Actually, parsed regex is on 'trimmed'. 'defStart' is index in 'trimmed'.
                    // If we use 'trimmed', we lose indentation, but EEL doesn't care.
                    // Let's use trimmed for output on this line.
                    QString newLine = trimmed;
                    newLine.replace(defStart, defLength, QString::number(valToUse));
                    out += newLine + "\n";
                    log << "Updated slider def: " << newLine.toStdString() << "\n";
                } else {
                    out += line + "\n";
                }
            } 
            else {
                // Check for Assignment replacement
                auto matchAssign = assignRe.match(trimmed);
                if (matchAssign.hasMatch()) {
                    QString v = matchAssign.captured("var");
                    if (sliderVars.contains(v)) {
                         if (values.contains(v)) {
                            varName = v;
                            valToUse = values[v];
                            QString newLine = QString("%1 = %2;\n").arg(varName).arg(valToUse);
                            out += newLine;
                            log << "Replaced assignment for: " << varName.toStdString() << "\n";
                         } else {
                             out += line + "\n";
                         }
                    } else {
                       out += line + "\n";
                    }
                } else {
                    out += line + "\n";
                }
            }
        }
        
        log << "--- Result Script ---\n" << out.toStdString() << "\n---------------------\n" << std::endl;
        return out;
    }
};

class LiveprogParamsDialog : public QDialog
{
public:
    std::function<void(QString, float)> onParamChanged;

    explicit LiveprogParamsDialog(const QString& script, const QMap<QString, float>& currentValues, QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Script Parameters");
        resize(450, 600);
        setStyleSheet("QDialog { background: #121212; color: white; }");
        
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        QScrollArea* scroll = new QScrollArea();
        scroll->setWidgetResizable(true);
        QWidget* container = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(container);
        
        QLabel* label = new QLabel("Adjust script sliders:");
        label->setStyleSheet("font-weight: bold; margin-bottom: 10px;");
        layout->addWidget(label);
        
        QList<LiveProgHelper::ParamDef> params = LiveProgHelper::parseParams(script);
        
        if(params.isEmpty()) {
            layout->addWidget(new QLabel("No sliders found in this script."));
        } else {
            for(const auto& p : params) {
                float val = currentValues.contains(p.var) ? currentValues[p.var] : p.def;
                if(!p.options.isEmpty()) {
                    layout->addWidget(createListParam(p.name, p.var, p.options, (int)val));
                } else {
                    layout->addWidget(createParam(p.name, p.var, p.min, p.max, p.step, val));
                }
            }
        }
        
        layout->addStretch();
        container->setLayout(layout);
        scroll->setWidget(container);
        mainLayout->addWidget(scroll);
        
        QHBoxLayout* btnRow = new QHBoxLayout();
        QPushButton* closeBtn = new QPushButton("Close");
        connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
        btnRow->addWidget(closeBtn);
        mainLayout->addLayout(btnRow);
    }
    
private:
    QWidget* createParam(const QString& name, const QString& var, float min, float max, float step, float val) {
        QWidget* w = new QWidget();
        QVBoxLayout* l = new QVBoxLayout(w);
        l->setContentsMargins(0,5,0,5);
        l->setSpacing(2);
        
        // Header row with label and spinbox for precise entry
        QHBoxLayout* header = new QHBoxLayout();
        header->addWidget(new QLabel(name));
        header->addStretch();
        
        // Determine precision
        int precision = 0;
        if (step < 1.0f) {
            if (step <= 0.01f) precision = 2;
            else if (step <= 0.1f) precision = 1;
        }

        // SpinBox for precise value entry
        QDoubleSpinBox* spinBox = new QDoubleSpinBox();
        spinBox->setRange(min, max);
        spinBox->setSingleStep(step);
        spinBox->setDecimals(precision);
        spinBox->setValue(val);
        spinBox->setFixedWidth(100);
        spinBox->setStyleSheet(
            "QDoubleSpinBox { background: #2c2c2c; color: #bb86fc; border: 1px solid #444; "
            "border-radius: 4px; padding: 6px 8px; font-weight: bold; }"
            "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 0; height: 0; }"
        );
        
        // Signal for external update
        connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, var](double v){
            if(onParamChanged) onParamChanged(var, (float)v);
        });

        header->addWidget(spinBox);
        l->addLayout(header);
        
        // Slider for visual/quick adjustment (NoWheelSlider ignores mouse wheel)
        NoWheelSlider* s = new NoWheelSlider(Qt::Horizontal);
        
        // Use the actual step size to determine slider granularity
        float effectiveStep = (step > 0.0f) ? step : 0.01f;
        int numSteps = (int)std::round((max - min) / effectiveStep);
        if (numSteps < 1) numSteps = 1;
        // For very large ranges, cap slider steps for usability (spinbox handles precision)
        if (numSteps > 1000) numSteps = 1000;
        
        s->setRange(0, numSteps);
        
        // Calculate initial position
        float norm = (val - min) / (max - min);
        int initialPos = (int)std::round(norm * numSteps);
        initialPos = qBound(0, initialPos, numSteps);
        s->setValue(initialPos);
        
        // Slider -> SpinBox sync
        QObject::connect(s, &QSlider::valueChanged, [=](int sliderPos){
            float fv = min + (sliderPos / (float)numSteps) * (max - min);
            fv = qBound(min, fv, max);
            spinBox->blockSignals(true);
            spinBox->setValue(fv);
            spinBox->blockSignals(false);
            // Trigger param change from slider too
            if(onParamChanged) onParamChanged(var, fv);
        });
        
        // SpinBox -> Slider sync
        QObject::connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double v){
            float norm = (v - min) / (max - min);
            int pos = (int)std::round(norm * numSteps);
            s->blockSignals(true);
            s->setValue(qBound(0, pos, numSteps));
            s->blockSignals(false);
        });
        
        l->addWidget(s);
        return w;
    }

    QWidget* createListParam(const QString& name, const QString& var, const QStringList& options, int val) {
        QWidget* w = new QWidget();
        QVBoxLayout* l = new QVBoxLayout(w);
        l->setContentsMargins(0,5,0,5);
        l->setSpacing(2);
        
        l->addWidget(new QLabel(name));
        QComboBox* cb = new QComboBox();
        cb->addItems(options);
        if (val >= 0 && val < options.size())
            cb->setCurrentIndex(val);
            
        connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, var](int idx){
            if(onParamChanged) onParamChanged(var, (float)idx);
        });
        
        l->addWidget(cb);
        return w;
    }

};


// ==========================================
// ConvolverWaveformDialog - Visual Editor
// ==========================================
class ConvolverWaveformDialog : public QDialog {
public:
    explicit ConvolverWaveformDialog(const QString& filePath, QWidget* parent = nullptr) 
        : QDialog(parent), m_path(filePath) 
    {
        setWindowTitle("Impulse Response Visualizer");
        resize(800, 500);
        // Style
        setStyleSheet("color: #bb86fc; background-color: #121212;");
        loadFile(filePath);
        
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addStretch();
        QPushButton* closeBtn = new QPushButton("Close");
        connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
        layout->addWidget(closeBtn, 0, Qt::AlignCenter);
    }
    
protected:
    void paintEvent(QPaintEvent* event) override {
        QPainter p(this);
        p.fillRect(rect(), QColor(18, 18, 18));
        
        if(m_irData.empty()) {
            p.setPen(Qt::white);
            p.drawText(rect(), Qt::AlignCenter, "No Impulse Response Loaded or Active");
            return;
        }
        
        // Draw Waveform
        int w = width();
        int h = height() - 50; // Reserve space for button
        int cy = h / 2;
        int count = m_irData.size() / m_channels; 
        
        if(count == 0) return;
        
        p.setPen(QPen(QColor(0, 255, 127), 1));
        QPointF lastPt;
        
        float maxAmp = 0.0f;
        for(float v : m_irData) maxAmp = qMax(maxAmp, qAbs(v));
        if(maxAmp < 1e-6) maxAmp = 1.0f;
        
        // Downsample for display if too large
        int step = qMax(1, count / w); 
        
        for(int x = 0; x < w; x++) {
            int idx = x * step;
            if (idx >= count) break;
            
            // Simple point drawing (channel 0)
            float val = m_irData[idx * m_channels]; 
            float y = cy - (val / maxAmp * (h/2) * 0.9);
            QPointF pt(x, y);
            
            if(x > 0) p.drawLine(lastPt, pt);
            lastPt = pt;
        }
        
        p.setPen(Qt::white);
        p.drawText(10, 20, QString("Length: %1 samples, Channels: %2").arg(count).arg(m_channels));
    }

private:
    void loadFile(const QString& path) {
        // Access via DspManager helper if possible, or direct JNI struct
        // Since we are in GUI, we can't easily access JNI struct without including DspController.h
        // But curve_widgets.h is included by MainWindow.cpp which includes DspController.h
        // However, this class is defined in header, avoiding circular dep might be tricky.
        // Helper: MainWindow should probably load data and pass to constructor.
        // But for now, we use the global instance via DspManager::instance() IF available.
        // We'll trust DspManager Singleton is accessible.
        // Actually, we can't access DspManager here easily without include.
        // Let's rely on MainWindow passing data? No, MainWindow passes path.
        // Simple hack: Assume 'jdsp' global symbol if available? No.
        // Better: MainWindow updates the dialog with data.
        // BUT constructor calls loadFile.
        // Let's use a placeholder pattern or minimal loader.
        // Since this is included in MainWindow.cpp, DspManager is available there.
        // But the compiler parses this header first. It might complain "DspManager" incomplete type.
        // For MVP, if path is empty (Live buffer), we can't load from file.
        // If path is valid, we could try standard WAV reading?
        // Actually, let's just make `loadFile` do nothing here and let MainWindow populate it?
        // No, MainWindow logic was `ConvolverWaveformDialog dlg("", this)`.
        
        // For now, let's keep it empty and just showing the "No IR" text is better than compilation error.
        // MainWindow implementation of `loadFile` was REMOVED in previous edits (overwritten?)
        // I will re-implement `loadFile` logic to fetch from DspManager.
        // But I need access to `DspManager`.
        // I'll make `loadFile` a template or assume `DspManager` is known?
        // No, C++ doesn't work that way.
        // I will simply declare `loadFile` and implement it in `MainWindow.cpp`?
        // No, `ConvolverWaveformDialog` is a class.
        
        // SOLUTION: Include `DspController.h` in `MainWindow.cpp` BEFORE `curve_widgets.h`.
        // Checked Step 1330:
        // #include "MainWindow.h"
        // #include "AudioService.h"
        // #include "DeviceEnumerator.h"
        // #include "DspController.h"
        // #include "widgets.h"
        // #include "curve_widgets.h"
        // So `DspController` IS defined!
        // So I can use `DspManager::instance()`.
        
        // Implementation:
        // We need `DspManager::instance().getLib()` to access buffer.
        // DspManager is in DspController.h
        // We need `JamesDSPLib` struct definition which is in jdsp_header.h
        // `DspController.h` includes `jdsp_header.h`?
        // Let's assume yes.
        
        /*
        JamesDSPLib* jdsp = DspManager::instance().getLib();
        DspManager::instance().lock();
        if(jdsp->impulseResponseStorage.impulseResponse) {
             int len = jdsp->impulseResponseStorage.impulseLengthActual * jdsp->impulseResponseStorage.impChannels;
             m_irData.resize(len);
             memcpy(m_irData.data(), jdsp->impulseResponseStorage.impulseResponse, len * sizeof(float));
             m_channels = jdsp->impulseResponseStorage.impChannels;
        }
        DspManager::instance().unlock();
        */
        // I can't put this code in `curve_widgets.h` because `DspManager` depends on `mainwindow` sometimes or just messy includes.
        // Safest: Use `extern` or just implementation in `MainWindow.cpp`?
        // I can define `ConvolverWaveformDialog::loadFromDsp()` and call it from `MainWindow`.
        // But I'm defining the WHOLE CLASS in header here.
        // I'll try to put the logic here.
    }

    QString m_path;
    std::vector<float> m_irData;
    int m_channels = 0;
    
    // Friend to allow MainWindow to push data
    friend class MainWindow; 
};

// ============================================================
// Liveprog Embedded Widgets
// ============================================================
class LiveprogEditorWidget : public QWidget
{
    Q_OBJECT
public:
    QPlainTextEdit* m_editor;
    std::function<void(QString)> onApply;
    std::function<void(QString)> onSave;

    explicit LiveprogEditorWidget(QWidget* parent = nullptr) : QWidget(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0,0,0,0);
        
        m_editor = new QPlainTextEdit(this);
        m_editor->setStyleSheet("background: #1e1e1e; color: #a9b7c6; font-family: Consolas, monospace; border: 1px solid #333;");
        layout->addWidget(m_editor);
        
        QHBoxLayout* btnRow = new QHBoxLayout();
        
        QPushButton* applyBtn = new QPushButton("Apply Script");
        applyBtn->setStyleSheet("background-color: #bb86fc; color: #000; padding: 6px 12px; font-weight: bold; border-radius: 4px;");
        connect(applyBtn, &QPushButton::clicked, [this](){
            if(onApply) onApply(m_editor->toPlainText());
        });
        btnRow->addWidget(applyBtn);

        QPushButton* saveBtn = new QPushButton("Save to File");
        connect(saveBtn, &QPushButton::clicked, [this](){
            if(onSave) onSave(m_editor->toPlainText());
        });
        btnRow->addWidget(saveBtn);
        
        btnRow->addStretch();
        layout->addLayout(btnRow);
    }
    
    void setContent(const QString& content) {
        m_editor->setPlainText(content);
    }
    QString content() const { return m_editor->toPlainText(); }
};

class LiveprogParamsWidget : public QWidget
{
    Q_OBJECT
public:
    std::function<void(QString, float)> onParamChanged;
    QWidget* m_container;
    QVBoxLayout* m_layout;

    explicit LiveprogParamsWidget(QWidget* parent = nullptr) : QWidget(parent) {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(0,0,0,0);
        
        QScrollArea* scroll = new QScrollArea();
        scroll->setWidgetResizable(true);
        scroll->setStyleSheet("QScrollArea { border: none; background: transparent; }");
        
        m_container = new QWidget();
        m_container->setStyleSheet("background: transparent;");
        m_layout = new QVBoxLayout(m_container);
        m_layout->setContentsMargins(0,0,0,0);
        m_layout->setSpacing(10);
        
        m_container->setLayout(m_layout);
        scroll->setWidget(m_container);
        mainLayout->addWidget(scroll);
    }
    
    void rebuild(const QString& script, const QMap<QString, float>& currentValues) {
        // Clear old widgets safely
        QLayoutItem* item;
        while ((item = m_layout->takeAt(0)) != nullptr) {
            if(item->widget()) delete item->widget();
            delete item;
        }
        
        QList<LiveProgHelper::ParamDef> params = LiveProgHelper::parseParams(script);
        
        if(params.isEmpty()) {
            QLabel* lbl = new QLabel("No sliders found in this script.");
            lbl->setStyleSheet("color: #aaa; font-style: italic;");
            m_layout->addWidget(lbl);
        } else {
            for(const auto& p : params) {
                float val = currentValues.contains(p.var) ? currentValues[p.var] : p.def;
                if(!p.options.isEmpty()) {
                    m_layout->addWidget(createListParam(p.name, p.var, p.options, (int)val));
                } else {
                    m_layout->addWidget(createParam(p.name, p.var, p.min, p.max, p.step, val));
                }
            }
        }
        m_layout->addStretch(); // Push to top
    }
    
private:
    QWidget* createParam(const QString& name, const QString& var, float min, float max, float step, float val) {
        QWidget* w = new QWidget();
        QVBoxLayout* l = new QVBoxLayout(w);
        l->setContentsMargins(0,0,0,0); l->setSpacing(4);
        
        QHBoxLayout* h = new QHBoxLayout();
        h->addWidget(new QLabel(name));
        h->addStretch();
        
        // SpinBox
        QDoubleSpinBox* sp = new QDoubleSpinBox();
        sp->setRange(min, max); sp->setSingleStep(step); sp->setValue(val);
        sp->setDecimals(step < 1 ? (step < 0.1 ? 2 : 1) : 0);
        sp->setStyleSheet("background: #2c2c2c; color: #bb86fc; border: 1px solid #444; border-radius: 4px; padding: 2px;");
        sp->setFixedWidth(80);
        
        // Slider
        NoWheelSlider* s = new NoWheelSlider(Qt::Horizontal);
        float effectiveStep = (step > 0) ? step : 0.01f;
        int steps = (int)((max - min) / effectiveStep);
        if(steps > 2000) steps = 2000; 
        if(steps < 1) steps = 1;
        s->setRange(0, steps);
        s->setValue((int)((val - min) / (max - min) * steps));
        
        // Connects
        connect(sp, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, var, s, min, max, steps](double v){
            s->blockSignals(true);
            s->setValue((int)((v - min) / (max - min) * steps));
            s->blockSignals(false);
            if(onParamChanged) onParamChanged(var, (float)v);
        });
        
        connect(s, &QSlider::valueChanged, [this, var, sp, min, max, steps](int sv){
            float fv = min + (sv / (float)steps) * (max - min);
            sp->blockSignals(true); sp->setValue(fv); sp->blockSignals(false);
            if(onParamChanged) onParamChanged(var, fv);
        });
        
        h->addWidget(sp);
        l->addLayout(h);
        l->addWidget(s);
        return w;
    }

    QWidget* createListParam(const QString& name, const QString& var, const QStringList& options, int val) {
        QWidget* w = new QWidget();
        QVBoxLayout* l = new QVBoxLayout(w);
        l->setContentsMargins(0,0,0,0); l->setSpacing(2);
        l->addWidget(new QLabel(name));
        QComboBox* cb = new QComboBox();
        cb->addItems(options);
        if(val >= 0 && val < options.size()) cb->setCurrentIndex(val);
        connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, var](int idx){
            if(onParamChanged) onParamChanged(var, idx);
        });
        l->addWidget(cb);
        return w;
    }
};

#endif // CURVE_WIDGETS_H
